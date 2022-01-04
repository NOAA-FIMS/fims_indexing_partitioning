remotes::install_github("https://github.com/NOAA-FIMS/fims_indexing_partitioning.git")
# 
# Example of FIMS indexing and partitioning through an R interface.
#

library( Rcpp )
library(FIP)

#load the module
fims_indexing <- Rcpp::Module("fims", dyn.load( "/Library/Frameworks/R.framework/Versions/4.0/Resources/library/FIP/libs/FIP.so" ))

#set timing
nyears<-30
nseasons<-3
nage<-10
nsexes<-2

#create time-varying seasons offset list
season_offsets<-list()
for(i in 1:nyears){
  offset<-c(1,2,3,4)
  season_offsets[[i]]<-offset/4
}

#create and area
area<-new(fims_indexing$area, nyears,season_offsets, nage)

#create a population
population<-new(fims_indexing$population,  nyears, season_offsets, nage)

#add an area for this population
population$add_area(area$id())

#initialize a 2 sex model
population$initialize_subpopulations(nsexes)

#evaluate (fake)
population$evaluate_subpopulations()

