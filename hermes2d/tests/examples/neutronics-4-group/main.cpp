#define HERMES_REPORT_WARN
#define HERMES_REPORT_INFO
#define HERMES_REPORT_VERBOSE
#define HERMES_REPORT_FILE "application.log"

#include "hermes2d.h"

// This test makes sure that example "neutronics-4-group" works correctly.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int INIT_REF_NUM = 2;                               // Number of initial uniform mesh refinements.
const int P_INIT_1 = 2,                                   // Initial polynomial degree for approximation of group 1 fluxes.
          P_INIT_2 = 3,                                   // Initial polynomial degree for approximation of group 2 fluxes.
          P_INIT_3 = 3,                                   // Initial polynomial degree for approximation of group 3 fluxes.
          P_INIT_4 = 4;                                   // Initial polynomial degree for approximation of group 4 fluxes.
const double ERROR_STOP = 1e-5;                           // Tolerance for the eigenvalue.
const MatrixSolverType matrix_solver = SOLVER_UMFPACK;    // Possibilities: SOLVER_UMFPACK, SOLVER_PETSC,
                                                          // SOLVER_MUMPS, and more are coming.
const char* iterative_method = "bicgstab";                // Name of the iterative method employed by AztecOO (ignored
                                                          // by the other solvers). 
                                                          // Possibilities: gmres, cg, cgs, tfqmr, bicgstab.
const char* preconditioner = "jacobi";                    // Name of the preconditioner employed by AztecOO (ignored by
                                                          // the other solvers). 
                                                          // Possibilities: none, jacobi, neumann, least-squares, or a
                                                          //  preconditioner from IFPACK (see solver/aztecoo.h)


// Initial eigenvalue approximation.
double k_eff = 1.140;         

// Element markers.
const int marker_reflector = 1;
const int marker_core = 2;

// Boundary markers.
const int bc_vacuum = 1;
const int bc_sym = 2;

// Boundary condition types.
BCType bc_types(int marker)
{
  return BC_NATURAL;
}

// Essential (Dirichlet) boundary condition values.
scalar essential_bc_values(int ess_bdy_marker, double x, double y)
{
  return 0;
}

// Physical data of the problem for the 4 energy groups.
#include "physical_parameters.cpp"
// Weak forms.
#include "forms.cpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Source function.
void source_fn(int n, Tuple<scalar*> values, scalar* out)
{
  for (int i = 0; i < n; i++)
  {
		out[i] = (nu[1][0] * Sf[1][0] * values.at(0)[i] +
        nu[1][1] * Sf[1][1] * values.at(1)[i] +
        nu[1][2] * Sf[1][2] * values.at(2)[i] +
        nu[1][3] * Sf[1][3] * values.at(3)[i]);
  }
}

// Integral over the active core.
double integrate(MeshFunction* sln, int marker)
{
  Quad2D* quad = &g_quad_2d_std;
  sln->set_quad_2d(quad);

  double integral = 0.0;
  Element* e;
  Mesh* mesh = sln->get_mesh();

  for_all_active_elements(e, mesh)
  {
    if (e->marker == marker)
    {
      update_limit_table(e->get_mode());
      sln->set_active_element(e);
      RefMap* ru = sln->get_refmap();
      int o = sln->get_fn_order() + ru->get_inv_ref_order();
      limit_order(o);
      sln->set_quad_order(o, H2D_FN_VAL);
      scalar *uval = sln->get_fn_values();
      double* x = ru->get_phys_x(o);
      double result = 0.0;
      h1_integrate_expression(x[i] * uval[i]);
      integral += result;
    }
  }

  return 2.0 * M_PI * integral;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// General testing class. It can test
//   > if the computed and expected results agree to within a specified tolerance (allowed_delta),
//   > if the computed results do not overshoot the expected results by more than allowed_delta,
//   > if the computed results do not undershoot the expected results by more than allowed_delta.
// When, for the first time, the expected behavior is not achieved, 'passed' is set to false and nothing more is tested.
// If used with a user-defined type, its class must overload all operators appearing in the intended test method.
template<typename T>
struct TestSubject
{
  T allowed_delta;
  bool passed;
  
  TestSubject(T allowed_delta) : allowed_delta(allowed_delta), passed(true) {};
  
  void test_equality(const T& computed, const T& expected) { 
    if(passed) passed = (computed <= expected + allowed_delta && computed >= expected - allowed_delta);
  }
  void test_overshoot(const T& computed, const T& expected) { 
    if(passed) passed = (computed <= expected + allowed_delta);
  }
  void test_undershoot(const T& computed, const T& expected) { 
    if(passed) passed = (computed >= expected - allowed_delta);
  }
};

// Structure that contains information about an extremum. It will be used
// for testing the equality of computed and expected maxima, therefore it
// overloads operators +,-,>=,<=..
struct Extremum
{
  double val, x, y;
  
  Extremum(double val, double x, double y) : val(val), x(x), y(y) {};
  
  inline Extremum operator+ (const Extremum &ex) const {
    return Extremum(val + ex.val, x + ex.x, y + ex.y);
  }
  inline Extremum operator- (const Extremum &ex) const {
    return Extremum(val - ex.val, x - ex.x, y - ex.y);
  }
  inline bool operator>= (const Extremum &ex) const {
    return (val >= ex.val && x >= ex.x && y >= ex.y);
  }
  inline bool operator<= (const Extremum &ex) const {
    return (val <= ex.val && x <= ex.x && y <= ex.y);
  }
  
  std::string str() { 
    char ret[50];
    sprintf(ret, "%lf, at (%lf,%lf)", val, x, y);
    return std::string(ret);
  }
};

// Calculates maximum of a given function, including its coordinates.
Extremum get_peak(MeshFunction *sln)
{
  Quad2D* quad = &g_quad_2d_std;
  sln->set_quad_2d(quad);
  Element* e;
  Mesh* mesh = sln->get_mesh();
  
  scalar peak = 0.0;
  double pos_x = 0.0;
  double pos_y = 0.0;
  
  for_all_active_elements(e, mesh)
  {
    update_limit_table(e->get_mode());
    sln->set_active_element(e);
    RefMap* ru = sln->get_refmap();
    int o = sln->get_fn_order() + ru->get_inv_ref_order();
    limit_order(o);
    sln->set_quad_order(o, H2D_FN_VAL);
    scalar *uval = sln->get_fn_values();
    int np = quad->get_num_points(o);
    double* x = ru->get_phys_x(o);
    double* y = ru->get_phys_y(o);
    
    for (int i = 0; i < np; i++)
      if (uval[i] > peak) {
        peak = uval[i];
        pos_x = x[i];
        pos_y = y[i];
      }
  }
  
  return Extremum(peak, pos_x, pos_y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  // Load the mesh.
  Mesh mesh;
  H2DReader mloader;
  mloader.load("reactor.mesh", &mesh);

  // Perform initial mesh refinements.
  for (int i = 0; i < INIT_REF_NUM; i++) mesh.refine_all_elements();

  // Solution variables.
  Solution sln1, sln2, sln3, sln4;
  Solution iter1, iter2, iter3, iter4;
  Tuple<Solution*> solutions(&sln1, &sln2, &sln3, &sln4);

  // Define initial conditions.
  info("Setting initial conditions.");
  iter1.set_const(&mesh, 1.00);
  iter2.set_const(&mesh, 1.00);
  iter3.set_const(&mesh, 1.00);
  iter4.set_const(&mesh, 1.00);

  // Create H1 spaces with default shapesets.
  H1Space space1(&mesh, bc_types, essential_bc_values, P_INIT_1);
  H1Space space2(&mesh, bc_types, essential_bc_values, P_INIT_2);
  H1Space space3(&mesh, bc_types, essential_bc_values, P_INIT_3);
  H1Space space4(&mesh, bc_types, essential_bc_values, P_INIT_4);
  Tuple<Space*> spaces(&space1, &space2, &space3, &space4);
  
  int ndof = Space::get_num_dofs(Tuple<Space*>(&space1, &space2, &space3, &space4));
  info("ndof = %d.", ndof);
  
  // Initialize the weak formulation.
  WeakForm wf(4);
  wf.add_matrix_form(0, 0, callback(biform_0_0), HERMES_SYM);
  wf.add_matrix_form(1, 1, callback(biform_1_1), HERMES_SYM);
  wf.add_matrix_form(1, 0, callback(biform_1_0));
  wf.add_matrix_form(2, 2, callback(biform_2_2), HERMES_SYM);
  wf.add_matrix_form(2, 1, callback(biform_2_1));
  wf.add_matrix_form(3, 3, callback(biform_3_3), HERMES_SYM);
  wf.add_matrix_form(3, 2, callback(biform_3_2));
  wf.add_vector_form(0, callback(liform_0), marker_core, Tuple<MeshFunction*>(&iter1, &iter2, &iter3, &iter4));
  wf.add_vector_form(1, callback(liform_1), marker_core, Tuple<MeshFunction*>(&iter1, &iter2, &iter3, &iter4));
  wf.add_vector_form(2, callback(liform_2), marker_core, Tuple<MeshFunction*>(&iter1, &iter2, &iter3, &iter4));
  wf.add_vector_form(3, callback(liform_3), marker_core, Tuple<MeshFunction*>(&iter1, &iter2, &iter3, &iter4));
  wf.add_matrix_form_surf(0, 0, callback(biform_surf_0_0), bc_vacuum);
  wf.add_matrix_form_surf(1, 1, callback(biform_surf_1_1), bc_vacuum);
  wf.add_matrix_form_surf(2, 2, callback(biform_surf_2_2), bc_vacuum);
  wf.add_matrix_form_surf(3, 3, callback(biform_surf_3_3), bc_vacuum);

  // Initialize the FE problem.
  bool is_linear = true;
  DiscreteProblem dp(&wf, spaces, is_linear);
 
  initialize_solution_environment(matrix_solver, argc, argv);
  
  SparseMatrix* matrix = create_matrix(matrix_solver);
  Vector* rhs = create_vector(matrix_solver);
  Solver* solver = create_linear_solver(matrix_solver, matrix, rhs);

  if (matrix_solver == SOLVER_AZTECOO) 
  {
    ((AztecOOSolver*) solver)->set_solver(iterative_method);
    ((AztecOOSolver*) solver)->set_precond(preconditioner);
    // Using default iteration parameters (see solver/aztecoo.h).
  }
  
  // Time measurement.
  TimePeriod cpu_time, solver_time;
  
  // Main power iteration loop:
  int iter = 1; bool done = false;
  bool rhs_only = false;
  do
  {
    info("------------ Power iteration %d:", iter);

    info("Assembling the stiffness matrix and right-hand side vector.");
    dp.assemble(matrix, rhs, rhs_only);
    
    info("Solving the matrix problem by %s.", MatrixSolverNames[matrix_solver].c_str());
    solver_time.tick(HERMES_SKIP);  
    bool solved = solver->solve();  
    solver_time.tick();
    
    if(solved)
      Solution::vector_to_solutions(solver->get_solution(), spaces, solutions);
    else
      error ("Matrix solver failed.\n");

    SimpleFilter source(source_fn, Tuple<MeshFunction*>(&sln1, &sln2, &sln3, &sln4));
    SimpleFilter source_prev(source_fn, Tuple<MeshFunction*>(&iter1, &iter2, &iter3, &iter4));

    // Compute eigenvalue.
    double k_new = k_eff * (integrate(&source, marker_core) / integrate(&source_prev, marker_core));
    info("Largest eigenvalue: %.8g, rel. difference from previous it.: %g", k_new, fabs((k_eff - k_new) / k_new));
    
    // Stopping criterion.
    if (fabs((k_eff - k_new) / k_new) < ERROR_STOP) done = true;

    // Update eigenvalue.
    k_eff = k_new;
    
    if (!done)
    {
      // Save solutions for the next iteration.
      iter1.copy(&sln1);    
      iter2.copy(&sln2);
      iter3.copy(&sln3);    
      iter4.copy(&sln4);
      
      // Don't need to reassemble the system matrix in further iterations,
      // only the rhs changes to reflect the progressively updated source.
      rhs_only = true;

      iter++;
    }
  }
  while (!done);
  
  // Time measurement.
  cpu_time.tick();
  solver_time.tick(HERMES_SKIP);
  
  // Print timing information.
  verbose("Average solver time for one power iteration: %g s", solver_time.accumulated() / iter);
  
  // Clean up.
  delete matrix;
  delete rhs;
  delete solver;
  
  finalize_solution_environment(matrix_solver);

  // Print timing information.
  verbose("Total running time: %g s", cpu_time.accumulated());
  
  // Calculation results for testing.
  info("Number of iterations: %d", iter);
  
  // Pointwise results.
  Extremum max1 = get_peak(&sln1); 
  Extremum max2 = get_peak(&sln2);
  Extremum max3 = get_peak(&sln3);
  Extremum max4 = get_peak(&sln4);
  
  info("Peak flux in group 1: %s", max1.str().c_str()); 
  info("Peak flux in group 2: %s", max2.str().c_str());
  info("Peak flux in group 3: %s", max3.str().c_str());
  info("Peak flux in group 4: %s", max4.str().c_str());
  
  // Integral results.
  info("Core eigenvalue: %lf", k_eff);
  
  #define ERROR_SUCCESS                                0
  #define ERROR_FAILURE                               -1
  
  TestSubject<int> num_iter(2);
  num_iter.test_overshoot(iter, 48);
  
  TestSubject<Extremum> peak(Extremum(1e-3, 1e-3, 1e-3));
  peak.test_equality(max1, Extremum(1.030123,1.665339,5.445262));
  peak.test_equality(max2, Extremum(2.262864,1.764661,5.445262));
  peak.test_equality(max3, Extremum(0.335336,1.764661,5.445262));
  peak.test_equality(max4, Extremum(4.565734,1.151700,5.494142));
  
  TestSubject<double> eigenvalue(1e-5);
  eigenvalue.test_equality(k_eff, 1.140916);
  
  if (num_iter.passed && peak.passed && eigenvalue.passed) {
    printf("Success!\n");
    return ERROR_SUCCESS;
  }
  else {
    printf("Failure!\n");
    return ERROR_FAILURE;
  }
}
