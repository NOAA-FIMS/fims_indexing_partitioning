remotes::install_github("https://github.com/NOAA-FIMS/fims_indexing_partitioning.git")

library( Rcpp )
library(FIP)

fims_indexing <- Rcpp::Module("fims", dyn.load( "/Library/Frameworks/R.framework/Versions/4.0/Resources/library/FIP/libs/FIP.so" ))

nyears<-30
nage<-10

season_offsets<-list()

for(i in 1:nyears){
  offset<-c(1,2,3,4)
  season_offsets[[i]]<-offset/4
}

population<-new(fims_indexing$population,  nyears, season_offsets, nage)
fims_indexing$say()
