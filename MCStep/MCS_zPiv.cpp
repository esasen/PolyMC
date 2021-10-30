#include "MCS_zPiv.h"


MCS_zPiv::MCS_zPiv(Chain * ch,const std::vector<long long int> & seedseq)
: MCStep(ch,seedseq)
{
    move_name="MCS_zPiv";
    // initialize range of random hinge selector
    decltype(genhinge.param()) new_range(1, num_bp-1);
    genhinge.param(new_range);

//    sigfac = 4.130;
    fac_sigma = 4;

    arma::mat covmat = chain->get_avg_cov();
    sigma = fac_sigma*1./3*(sqrt(covmat(0,0))+sqrt(covmat(1,1))+sqrt(covmat(2,2)));

    // intervals of moved segments for exluded volume interactions
    moved_intervals.push_back({0,num_bp-1,0});
    moved_intervals.push_back({-1,num_bp-1,1000});

    suitablility_key[MCS_FORCE_ACTIVE]              = true;  // force_active
    suitablility_key[MCS_TORQUE_ACTIVE]             = true;  // torque_active
    suitablility_key[MCS_TOPOLOGY_CLOSED]           = false; // topology_closed
    suitablility_key[MCS_FIXED_LINK]                = true;  // link_fixed
    suitablility_key[MCS_FIXED_TERMINI]             = true;  // fixed_termini
    suitablility_key[MCS_FIXED_TERMINI_RADIAL]      = true;  // fixed_termini_radial
    suitablility_key[MCS_FIXED_FIRST_ORIENTATION]   = true;  // fixed_first_orientation
    suitablility_key[MCS_FIXED_LAST_ORIENTATION]    = true;  // fixed_last_orientation

    requires_EV_check=true;
}

MCS_zPiv::MCS_zPiv(Chain * ch,const std::vector<long long int> & seedseq, int id1, int id2)
: MCStep(ch,seedseq)
/*
    id1, and id2 specify the (included) boundaries of the interval within which the pivot
    point can be selected. The selected point is the first triad to be rotated. The position
    of the monomer is not changed by the move.
*/
{
    move_name="MCS_zPiv";
    // initialize range of random hinge selector
    if (id2 >= num_bp || id2 < 0) {
        id2 = num_bp - 1;
    }
    if (id1 < 1) {
        id1 = 1;
    }
    decltype(genhinge.param()) new_range(id1,id2);
    genhinge.param(new_range);

//    sigfac = 4.130;
    fac_sigma = 4;

    arma::mat covmat = chain->get_avg_cov();
    sigma = fac_sigma*1./3*(sqrt(covmat(0,0))+sqrt(covmat(1,1))+sqrt(covmat(2,2)));

    // intervals of moved segments for exluded volume interactions
    moved_intervals.push_back({0,num_bp-1,0});
    moved_intervals.push_back({-1,num_bp-1,1000});

    suitablility_key[MCS_FORCE_ACTIVE]              = true;  // force_active
    suitablility_key[MCS_TORQUE_ACTIVE]             = true; // torque_active
    suitablility_key[MCS_TOPOLOGY_CLOSED]           = false; // topology_closed
    suitablility_key[MCS_FIXED_LINK]                = false; // link_fixed
    suitablility_key[MCS_FIXED_TERMINI]             = false; // fixed_termini
    suitablility_key[MCS_FIXED_TERMINI_RADIAL]      = false; // fixed_termini_radial
    suitablility_key[MCS_FIXED_FIRST_ORIENTATION]   = true; // fixed_first_orientation
    suitablility_key[MCS_FIXED_LAST_ORIENTATION]    = false; // fixed_last_orientation

    requires_EV_check=true;
}


MCS_zPiv::~MCS_zPiv() {

}

void MCS_zPiv::update_settings() {
    arma::mat covmat = chain->get_avg_cov();
    sigma = fac_sigma*1./3*(sqrt(covmat(0,0))+sqrt(covmat(1,1))+sqrt(covmat(2,2)));
}

bool MCS_zPiv::MC_move() {
    int    hingeID  = genhinge(gen);
    int    BPS_ID   = hingeID-1;
    double choice   = uniformdist(gen);
    double deltaE   = 0;

    double    z_phi = normaldist(gen) * sigma;
    arma::mat rot_mat;

    changed_bps={BPS_ID};
    rot_mat        = getRotMat(chain->get_force_dir()*z_phi);
    arma::mat Trot = rot_mat*triads->slice(hingeID);

    BPS[BPS_ID]->propose_move(triads->slice(hingeID-1),Trot);
    deltaE = BPS[BPS_ID]->eval_delta_energy();

	if (chain->force_active() == true) {
        // TODO: don_t assume that the first position is at (0,0,0)
		arma::colvec ETE_old = pos->col(num_bp-1);
		arma::colvec ETE_new = rot_mat*(pos->col(num_bp-1)-pos->col(hingeID)) + pos->col(hingeID);
        deltaE += arma::dot(chain->get_beta_force_vec(),ETE_old-ETE_new); //- arma::dot(chain->get_beta_force_vec(),ETE_new);
	}

	if (exp(-deltaE) <= choice) {
		return false;
    }
	else {
		triads->slice(hingeID) = rot_mat * triads->slice(hingeID);
        for (int j=hingeID+1;j<num_bp;j++) {
            triads->slice(j) = rot_mat * triads->slice(j);
            pos->col(j)      = pos->col(j-1) + triads->slice(j-1).col(2)*disc_len;
        }
        moved_intervals[0](EV_TO) = hingeID;
        moved_intervals[1](EV_FROM) = hingeID+1;
        // the last one is permanently set to num_bp-1
    }
    return true;
}


