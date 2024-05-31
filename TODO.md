- check_min_max (used in check_assignment)
- check_assignment (used in check_reduction)
- check_reduction
- parallelize
- parallize_reduction
- add #include <omp.h> directive

Probably not:

- remove struct types from alive/dead variables
- move analyze to util, call it return_candidate_variables(std::string);
