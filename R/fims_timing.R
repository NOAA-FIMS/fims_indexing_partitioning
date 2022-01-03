library(Rcpp)
#



fims <- Rcpp::Module("fims", PACKAGE="fims_indexing_partitioning")

population<-new(fims$population)
