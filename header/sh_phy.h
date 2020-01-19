struct sh_phy_force {
	float life_time; //-1 means infinity
	float force_amount;
	vec2  direction;
	sh_phy_force* n;
};

vec2 calculate_forces(sh_phy_force* forces, double time_delta);
void update_forces(sh_phy_force** forces);
void add_force(sh_phy_force** forces, sh_phy_force force_to_add);

