
inherit "obj/actor";

static object user;
static string prompt;
static mapping xverbs;
object *saved_items;

string query_prompt()
{
	if (!prompt) prompt = "> ";
	return prompt;
}

object query_user() { return user; }

void create()
{
	int i;
	restore_object(object_name(this_object()) + ".sav");
	if (saved_items)
	{
		for (i=sizeof(saved_items); i--;)
		{
			if (!saved_items[i]) continue;
			saved_items[i]->__move(this_object());
		}
	}
}

void set_user(object u)
{
	string s;
	if (previous_program() == User) {
		user = u;
		s = u->query_user_name();
		set_name(capitalize(s));
	}
}

void save()
{
	int i,x;
	saved_items = all_inventory();
	if (saved_items)
	{
		for (i=sizeof(saved_items); i--;)
		{
			x = 0;
			catch { x = saved_items[i]->query_save(); };
			if (!x) saved_items[i] = nil;
		}
	}
	save_object(object_name(this_object()) + ".sav");
	saved_items = nil;
}

void quit()
{
	int i;
	save();
	destruct();
}

int could_see(object o)
{
	if (!o->is_visible()) return 0;
	if (o->query_light_upon() <= 0) return 0;
	return 1;
}

/*
	Returns the array of objects in the inventory of 'o' that we can see,
 	Optionally filters by id(), or filters out this_object().
*/

object *visible_inventory(object o, varargs string id_to, int see_myself)
{
	object *inv;
	int i, sz, found;

	inv = o->all_inventory();
	if (!inv || !(sz=sizeof(inv))) return nil;
	for (i=0; i<sz; i++) {
		if (!inv[i]) continue;
		if (!could_see(inv[i])) continue;
		if (!see_myself && inv[i] == this_object()) continue;
		if (id_to && !inv[i]->id(id_to)) continue;
		/* okay, it's legit */
		if (i>found) inv[found] = inv[i];
		found++;
	}
	inv = inv[..found-1];
	return inv;
}

object find_by_id(object o, string name, varargs int see_myself)
{
	int i, nth, sz;
	string tmp;
	object *inv;

	if (!o) return nil;
	if (sscanf(name,"%s %d",tmp,nth) == 2)
		name = tmp;
	else
		nth = 1;

	inv = visible_inventory(o, name, see_myself);
	if (!inv || (i=sizeof(inv)) < nth) return nil;
	return inv[nth-1];
}

string look_inventory(object o)
{
	mixed *inv;
	string *ret, s;
	int i,sz,ct;

	if(!o) return "";
	inv = visible_inventory(o, nil);
	if (!inv || !(sz=sizeof(inv))) return "";
	for (i=0; i<sz; i++) {
		inv[i] = inv[i]->short();
	}
	if (sz == 1) return "You see " + inv[0] + " here.";
	if (sz == 2) return "You see " + inv[0] + " and " + inv[1] + " here.";
	inv[sz-1] = "and " + inv[sz-1];
	return "You see here: " + implode(inv, ", ") + ".";
}

int cmd_drop(string args)
{
	object o;
	o = find_by_id(this_object(), args);
	if (!o) return fail("You don't have " + an(args) + ".");
	o->__move(environment());
	write("You drop " + the(o) + ".");
	return 1;
}

int cmd_get(string args)
{
	string thing, place;
	object o, loc;

	if (sscanf(args,"%s from %s",thing,place) == 2) {
		loc = find_by_id(this_object(),place);
		if (!loc) loc = find_by_id(environment(),place);
		if (!loc) return fail("You don't see " + an(place) + " to get from.");
		o = find_by_id(loc, thing);
		if (!o) return fail("You don't see " + an(thing) + " in " + the(place) + ".");
	} else {
		loc = environment();
		o = find_by_id(loc,args);
		if (!o) return fail("You don't see " + an(args) + " here.");
	}
	o->__move(this_object());
	if (loc == environment())
		write("You pick up " + an(o) + ".");
	else
		write("You take " + the(o) + " from " + the(loc) + ".");
	return 1;
}

int cmd_put(string args)
{
	string thing, place;
	object o, loc;

	if (sscanf(args,"%s in %s",thing,place) == 2
			|| sscanf(args,"%s into %s",thing,place) ==2) {
		loc = find_by_id(this_object(), place);
		if (!loc) loc = find_by_id(environment(), place);
		if (!loc) return fail("You don't see "+an(place)+" to put anything.");
		o = find_by_id(this_object(), thing);
		if (!o) return fail("You don't have "+an(thing)+".");
		o->__move(loc);
		write("You put " + the(o) + " in " + the(loc) + ".");
		return 1;
	} else { return fail("Put what into what?"); }
}

int cmd_look(string args)
{
	object env,o;
	string s;

	env = environment();
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

int cmd_go(string dir)
{
	string dest; object loc;
	loc = environment();
	if (!loc) return fail("You can't go anywhere from nowhere!");
	loc = loc->query_exit(dir);
	if (!loc) return fail("You can't go " + dir + " from here.");
	__move(loc);
	write("You go " + dir);
	return 1;
}

mapping query_xverbs()
{
	return xverbs;
}

void add_xverb(string verb, string callback)
{
	mixed *val;
	int i;
	object o;

	o = previous_object();
	if (!xverbs) xverbs = ([ ]);
	val = xverbs[verb];
	if (!val)
	{
		xverbs[verb] = ({ o, callback });
		return;
	}
	for(i=sizeof(val)-2; i>=0; i-=2)
	{
		if (val[i] == o && val[i+1] == callback) return;
	}
	xverbs[verb] = val + ({ o, callback });
}

int do_xverbs(string verb, string args)
{
	mixed *cbs;
	int i,ret;
	object me,room;
	if (!xverbs) return 0;
	cbs = xverbs[verb];
	if (!cbs) return 0;
	me = this_object();
	room = me->environment();
	for (i=sizeof(cbs)-2; i>=0; i-=2)
	{
		object o; string func;
		o = cbs[i];
		func = cbs[i+1];
		if (o && (o->environment() == me || o == room || o->environment() == room))
		{
			ret = call_other(o,func,verb,args);
			if (ret) break;
		}
		else
		{
			cbs = cbs[..i-1] + cbs[i+2..];
		}
	}
	xverbs[verb] = cbs;
	return ret;
}

/* Called by our user object with an input string.
 * We should return 1 if we handled the command, 0 if we did not.
 */

int do_command(string str)
{
	int c, did_cmd;
	string cmd, args;

	cmd = str;

	set_fail_reason(nil);

	if (str == "") return 0;
	c = str[0];
	if (c == ':') str = "emote " + cmd[1..];
	else if (c == '\'' || c == '\"') str = "say " + cmd[1..];
	else if (c == '@') str = "tell " + cmd[1..];

	switch (cmd) {
		case "n": case "north": str = "go north"; break;
		case "s": case "south": str = "go south"; break;
		case "e": case "east": str = "go east"; break;
		case "w": case "west": str = "go west"; break;
		case "ne": case "northeast": str = "go northeast"; break;
		case "se": case "southeast": str = "go southeast"; break;
		case "sw": case "southwest": str = "go southwest"; break;
		case "nw": case "northwest": str = "go northwest"; break;
		case "u": case "up": str = "go up"; break;
		case "d": case "down": str = "go down"; break;
		case "in": case "enter": str = "go in"; break;
		case "out": case "exit": str = "go out"; break;
	}

	args = "";
	if (sscanf(str,"put on %s",args))
		cmd = "wear";
	else if (sscanf(str,"take off %s",args))
		cmd = "remove";
	else
		sscanf(str,"%s %s",cmd,args);

	write("(obj/player cmd='" + cmd + "' args='" + args + "')");

	switch (cmd) {
		case "take":
		case "get": did_cmd = cmd_get(args); break;
		case "put": did_cmd = cmd_put(args); break;
		case "drop": did_cmd = cmd_drop(args); break;
		case "x": case "exa": case "look":
		case "l": did_cmd = cmd_look(args); break;
		case "go": did_cmd = cmd_go(args); break;

	}

	if (!did_cmd)
	{
		did_cmd = do_xverbs(cmd,args);
	}

	if (did_cmd) return 1;

	/* todo look for commands elsewhere */



	str = get_fail_reason();
	if (str) write(str);
	else write("What?");

	return 1;
}
