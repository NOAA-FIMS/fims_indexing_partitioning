/* 
 * File:   fims_indexing.cpp
 * Author: Matthew Supernaw
 *
 * Created on December 17, 2021, 12:42 PM
 * 
 * A simple example to demonstrate object partitioning and time 
 * indexing for FIMS.
 * 
 */

#include <cstdlib>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <random>
#include <RcppCommon.h>
#include <Rcpp.h>
#include <typeinfo>
#warning "compiling example"

using namespace Rcpp;

/**
 *Base class, holds common modeling information.
 */
class model_base {
    static size_t id_g; //used to create unique identifier for model objects
public:
    size_t nyears_; //number of years
    size_t nseasons_; //number of seasons, equals seasons_max_ for variable season lengths
    size_t nages_; //number of ages
    std::vector<std::vector<double> > season_offsets_; //seasons offsets, entries can be fixed or variable
    size_t seasons_max_; //max seasons for all years
    size_t object_id; //objects unique identifier

    /**
     * Constructor for variable season data.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    model_base(size_t nyears, std::vector<std::vector<double> > season_offsets, size_t nages) :
    nyears_(nyears), season_offsets_(season_offsets), nages_(nages) {
        this->object_id = model_base::id_g++;
        seasons_max_ = 0;

        //find the max season size in the time series
        for (size_t i = 0; i < nyears; i++) {
            this->seasons_max_ = std::max(this->seasons_max_, this->season_offsets_[i].size());
        }

    }

    /**
     * Constructor for variable season data.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    model_base(size_t nyears, Rcpp::List season_offsets, size_t nages) :
    nyears_(nyears), nages_(nages) {
        this->object_id = model_base::id_g++;
        seasons_max_ = 0;
        Rcpp::List::iterator it;

        for (it = season_offsets.begin(); it != season_offsets.end(); it++) {

            std::vector<double> t = *it;
            this->season_offsets_.push_back(t);

        }



    }

    /**
     * Constructor for fixed season data.
     * 
     * @param nyears
     * @param nseasons
     * @param nages
     */
    model_base(size_t nyears, size_t nseasons, size_t nages) :
    nyears_(nyears), nseasons_(nseasons), nages_(nages) {
        this->object_id = model_base::id_g++;
        seasons_max_ = nseasons;
        season_offsets_.resize(nyears);
        for (int i = 0; i < nyears; i++) {
            for (double j = 0; j < nseasons; j++) {
                this->season_offsets_[i].push_back((j + 1.0) / static_cast<double> (nseasons));
            }
        }
    }

    /**
     * Return dimension folded index for time and age;
     * 
     * @param year
     * @param season
     * @param age
     * @return 
     */
    inline const size_t get_index(
            const size_t& year,
            const size_t& season,
            const size_t& age) {

        return year * this->seasons_max_ * this->nages_ + season * this->nages_ + age;
    }

    /**
     * Return dimension folded index for time only;
     * 
     * @param year
     * @param season
     * @return 
     */
    inline const size_t get_index(
            const size_t& year,
            const size_t& season) {

        return year * this->seasons_max_ * this->nages_ + season;
    }

    /**
     * Returns the number of seasons for a given year
     * 
     * @param year
     * @return 
     */
    inline const size_t get_seasons(const size_t& year) {
        return this->season_offsets_[year].size();
    }



};

size_t model_base::id_g = 0; //initialize the static id counter

/**
 * Area object
 */
class area : public model_base {
public:
    static std::map<size_t, area*> models;
    typedef typename std::map<size_t, area*>::iterator area_iterator;

    area(size_t nyears, size_t nseasons, size_t nages) :
    model_base(nyears, nseasons, nages) {
        area::models[this->object_id] = this;
    }

    area(size_t nyears, Rcpp::List season_offsets, size_t nages) :
    model_base(nyears, season_offsets, nages) {
        area::models[this->object_id] = this;
    }

    size_t id() {
        return this->object_id;
    }


};
std::map<size_t, area*> area::models;

/**
 * Base class for all population objects.
 * Inherits from model_base
 */
class population_base : public model_base {
public:
    std::vector<double> ages; //age classes for population objects

    /**
     * Constructor for fixed season size.
     * 
     * @param nyears
     * @param nseasons
     * @param nages
     * @param ages
     */
    population_base(size_t nyears, size_t nseasons, size_t nages, std::vector<double> ages) :
    model_base(nyears, nseasons, nages), ages(ages) {//initialize base class

    }

    /**
     * Constructor for variable season size.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    population_base(size_t nyears, std::vector<std::vector<double> > season_offsets, size_t nages) :
    model_base(nyears, season_offsets, nages) {//initialize base class
    }

    /**
     * Constructor for variable season size.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    population_base(size_t nyears, Rcpp::List season_offsets, size_t nages) :
    model_base(nyears, season_offsets, nages) {//initialize base class
    }


};

/**
 * Subpopulation partitioned by sex and area.
 * Inherits from population_base
 */
class subpopulation : public population_base {
public:

    std::vector<double> some_derived_quantities; //made up derived quantities

    /**
     * Constructor for fixed season size.
     * 
     * @param nyears
     * @param nseasons
     * @param nages
     * @param ages
     */
    subpopulation(size_t nyears, size_t nseasons, size_t nages, std::vector<double> ages) :
    population_base(nyears, nseasons, nages, ages) {//initialize base class
        some_derived_quantities.resize(nyears * this->seasons_max_ * nages);
    }

    /**
     * Constructor for variable season size.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    subpopulation(size_t nyears, std::vector<std::vector<double> > season_offsets, size_t nages) :
    population_base(nyears, season_offsets, nages) {//initialize base class
        some_derived_quantities.resize(nyears * this->seasons_max_ * nages);
    }

    /**
     * Constructor for variable season size.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    subpopulation(size_t nyears, Rcpp::List season_offsets, size_t nages) :
    population_base(nyears, season_offsets, nages) {//initialize base class
        some_derived_quantities.resize(nyears * this->seasons_max_ * nages);
    }

    std::shared_ptr<area> area_;

    void calculate_some_life_history_1(size_t index) {
        std::cout << "doing some life history stuff at index " << index << std::endl;
        this->some_derived_quantities[index] = index;
    }

    /**
     * Just print some stuff to simulate the finalization process.
     */
    void finalize() {

        std::cout << "subpopulation " << this->object_id << "\n\n";

        for (size_t y = 0; y < this->nyears_; y++) {
            for (size_t s = 0; s < this->get_seasons(y); s++) {
                std::cout << "subpopulation " << this->object_id << " " << "year " << y << " " << "season " << s << "\n";
                for (size_t a = 0; a < this->nages_; a++) {
                    size_t index = this->get_index(y, s, a);
                    std::cout << this->some_derived_quantities[index] << "  ";
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }

};

/**
 * Population class holds partitioned subpopulations.
 * Inherits from population_base
 */
class population : public population_base {
public:
    std::vector<std::shared_ptr<area> > areas_;


    //given a sex, hold a vector of area specific sub_populations, hashed by sex index.
    std::map<size_t, std::vector<std::shared_ptr<subpopulation> > > subpopulation_;

    size_t nsexes_;

    /**
     * Constructor for fixed season size.
     * 
     * @param nyears
     * @param nseasons
     * @param nages
     * @param ages
     */
    population(size_t nyears, size_t nseasons, size_t nages, std::vector<double> ages) :
    population_base(nyears, nseasons, nages, ages) {//initialize base class
    }

    /**
     * Constructor for variable season size.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    population(size_t nyears, std::vector<std::vector<double> > season_offsets, size_t nages) :
    population_base(nyears, season_offsets, nages) {//initialize base class
    }

    /**
     * Constructor for variable season size.
     * 
     * @param nyears
     * @param season_offsets
     * @param nages
     */
    population(size_t nyears, Rcpp::List season_offsets, size_t nages) :
    population_base(nyears, season_offsets, nages) {//initialize base class
    }

#ifdef NATIVE_BUILD

    /**
     * initialize subpopulations, partition by sex and area.
     * 
     * @param nsexes
     * @param areas
     */
    void initialize_subpopulations(const size_t& nsexes,
            const std::vector<std::shared_ptr<area> >& areas) {
        this->nsexes_ = nsexes;
        this->areas_ = areas;



        for (int i = 0; i < this->nsexes_; i++) {
            for (int j = 0; j < this->areas_.size(); j++) {
                std::shared_ptr<subpopulation> sub_pop = std::make_shared<subpopulation>(this->nyears_, this->season_offsets_, this->nages_);
                sub_pop->area_ = this->areas_[i];
                this->subpopulation_[i].push_back(sub_pop);
            }
        }

    }
#endif

    /**
     * initialize subpopulations, partition by sex and area.
     * 
     * @param nsexes
     * @param areas
     */
    void initialize_subpopulations(const size_t& nsexes) {
        this->nsexes_ = nsexes;
        std::cout << "initializing subpopulations of " << nsexes << ":\n\n";
        for (int i = 0; i < this->nsexes_; i++) {
            for (int j = 0; j < this->areas_.size(); j++) {
                std::cout << i << " --- " << j << "\n";
                std::shared_ptr<subpopulation> sub_pop = std::make_shared<subpopulation>(this->nyears_, this->season_offsets_, this->nages_);
                sub_pop->area_ = this->areas_[j];
                this->subpopulation_[i].push_back(sub_pop);
            }
        }

    }

    void add_area(int area_id) {

        typename area::area_iterator it;

        it = area::models.find(area_id);
        if (it != area::models.end()) {
            std::shared_ptr<area> area((*it).second); // = std::make_shared<area>(b.nyears_, b.nseasons_, b.nages_);
            this->areas_.push_back(area);
        }
    }

    /**
     * Loops through sex/area partitions and evaluates "some life history stuff"
     * based on modeling time step.
     * 
     */
    void evaulate_subpopulations() {

        for (size_t i = 0; i < this->nsexes_; i++) {
            std::vector<std::shared_ptr<subpopulation> >&
                    sub_pops = this->subpopulation_[i];
            std::cout<<"sex "<<i<<",  "<<std::flush;
            for (size_t j = 0; j < sub_pops.size(); j++) {
                std::cout<<"sub_pop "<<j<<",  "<<std::flush;
                for (size_t y = 0; y < this->nyears_; y++) {
                    std::cout<<"years "<<y<<",  "<<std::flush;
                    for (size_t s = 0; s < this->get_seasons(y); s++) {
                        std::cout<<"season "<<s<<",  "<<std::flush;
                        for (size_t a = 0; a < this->nages_; a++) {
                            std::cout<<"age "<<a<<"  "<<std::flush;
                            size_t index = this->get_index(y, s, a);
                            sub_pops[j]->calculate_some_life_history_1(index);
                        }
                    }
                }

            }
            std::cout<<std::endl;
        }
    }

    /**
     * Loops through sex/area partitions and calls the subpopulation finalize
     * method.
     */
    void finalize() {
        for (size_t i = 0; i < this->nsexes_; i++) {
            std::vector<std::shared_ptr<subpopulation> >&
                    sub_pops = this->subpopulation_[i];

            for (size_t j = 0; j < sub_pops.size(); j++) {
                sub_pops[j]->finalize();
            }
        }
    }
};

void say() {
    std::cout << "hi!";
}

RCPP_EXPOSED_CLASS(population)

RCPP_MODULE(fims) {
    using namespace Rcpp;

    class_<population >("population")
            .constructor<size_t, Rcpp::List, size_t>()
            .method("evaluate_subpopulations", &population::evaulate_subpopulations)
            .method("initialize_subpopulations", &population::initialize_subpopulations)
            .method("add_area", &population::add_area);

    class_<area >("area")
            .constructor<size_t, Rcpp::List, size_t>()
            .method("id", &area::id);
}

/*
 * 
 */
// int main(int argc, char** argv) {
//     std::vector<double> ages = {1, 2, 3, 4, 5, 6, 7, 8};
//     size_t years = 30;
//     size_t seasons = 4;

//     std::vector<std::shared_ptr<area> > areas = {std::make_shared<area>(years, seasons, ages.size()),
//         std::make_shared<area>(years, seasons, ages.size()),
//         std::make_shared<area>(years, seasons, ages.size())};

//     //1. Fixed Seasons Example.

//     //create a population with fixed seasons
//     population pop(years, seasons, ages.size(), ages);
//     pop.initialize_subpopulations(2, areas);
//     pop.evaulate_subpopulations();
//     pop.finalize();


//     //1. Variable Seasons Example.

//     //randomly generate a variable season matrix based on year
//     std::default_random_engine generator;
//     std::uniform_int_distribution<size_t> distribution(1, 4);

//     std::vector<std::vector<double> > season_offsets(years);
//     for (int y = 0; y < years; y++) {
//         std::cout << "year = " << y << "\n";
//         std::vector<double> seasons;

//         //randomly get the number of seasons for this year
//         size_t nseasons = distribution(generator);
//         for (int s = 0; s < nseasons; s++) {
//             //push back this season offset 
//             seasons.push_back((s + 1) / static_cast<double> (nseasons));
//             std::cout << seasons[s] << " ";
//         }
//         std::cout << std::endl;
//         //set the season offset vector for year y
//         season_offsets[y] = seasons;
//     }

//     //create a population with variable seasons
//     population pop2(years, season_offsets, ages.size());
//     pop2.initialize_subpopulations(2, areas);
//     pop2.evaulate_subpopulations();
//     pop2.finalize();


//     return 0;
// }

