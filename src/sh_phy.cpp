vec2 calculate_forces(sh_phy_force* forces, double time_delta) {
	vec2 result;
	while(forces) {
		result = result + forces->force_amount*normalize(forces->direction);
		forces->life_time -= time_delta;
		forces->force_amount -= forces->force_amount*time_delta;
		forces = forces->n;
	}

	return result;
}

void update_forces(sh_phy_force** forces) {
	while(forces != nullptr && *forces != nullptr) {

		if((*forces)->life_time <= 0) {
			sh_phy_force* to_delete = *forces;
			*forces = (*forces)->n;
			free(to_delete);
		}

		if((*forces) != nullptr)
			forces = &(*forces)->n;
	}

}

void add_force(sh_phy_force** forces, sh_phy_force force_to_add) {
	if(*forces == nullptr) {
		*forces = (sh_phy_force*) malloc(sizeof(sh_phy_force));
		(*forces)->life_time = force_to_add.life_time;
		(*forces)->direction = force_to_add.direction;
		(*forces)->force_amount = force_to_add.force_amount;
		(*forces)->n = nullptr;
	} else {


		while((*forces)->n != nullptr) {
			forces = &((*forces)->n);
		}

		sh_phy_force* n_f = (sh_phy_force*) malloc(sizeof(sh_phy_force));
		n_f->life_time = force_to_add.life_time;
		n_f->direction = force_to_add.direction;
		n_f->force_amount = force_to_add.force_amount;
		n_f->n = nullptr;

		(*forces)->n = n_f;
	}
}

