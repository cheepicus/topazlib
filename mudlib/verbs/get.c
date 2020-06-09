

int parse(object me, string args)
{
	string thing, box;
	object o,loc;

	if (sscanf(args,"%s from %s",thing,box) == 2)
	{
		loc = me->find_in(me,box);
		if (!loc) ob = me->find_in(me->environment(),box);
		if (!loc) return notify_fail("I don't see " + box + " here.");
	}
	else
	{
		thing = args;
	}
	if (!loc) loc = me->environment();
	o = me->find_in(loc,thing);

	ot = me->

}

int get(object me, string args)
{
	object env,o;
	string s;

	env = me->environment();
	if (!args || args == "") {
		if (!env) {
			write("You are in a swirling colorless void.");
			return 1;
		}
		if (env->query_total_light() <= 0) {
			write("It is too dark to see your surroundings.");
		} else {
			write("\033[1m" + env->short() + "\033[0m");
			write(env->long());
			s = env->exits();
			if (!s) s = "\tNo obvious exits.";
			else s = "\tObvious exits lead \033[1m" + s + "\033[0m.";
			write(s);
		}
		write(look_inventory(env));
		return 1;
	}
	if (starts_with(args,"at "))
		sscanf(args,"at %s",args);
	if (env) o = find_by_id(env, args,1);
	if (!o) o = find_by_id(this_object(), args);
	if (!o) return fail("You don't see " + an(args) + " here.");

	write(o->short());
	write(o->long());
	write(look_inventory(o));
	return 1;
}
