

/* Something that goes around acting upon the environment. */

inherit "obj/base.inh";

int is_actor() { return 1; }

static string fail_reason;
void set_fail_reason(string s) { fail_reason = s; }
string get_fail_reason() { return fail_reason; }

int my_vision; /* added to light we can see */

object find_in_inventory(string id_str, object container, varargs int nth)
{
	object *inv,o;
	int i;

	inv = container->inventory();
	if (!inv) return nil;
	if (!nth) nth = 1;
	for (i=0; i<sizeof(inv); i++)
	{
		o = inv[i];
		if (!o->visible()) continue;
		if (!o->id(id_str)) continue;
		if (o == this_object()) continue;
		nth--;
		if (!nth) return o;
	}
	return nil;
}


int can_see(object o)
{
	if (!o) o = environment();
	if (!o) return 0;
	if (!o->is_visible()) return 0;
	if (o->query_light_upon() <= 0) return 0;
	return 1;
}

/* what objects can we see, so that we know */

/* how much can we see in an object */

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



int do_command(string str)
{
	int c, did_cmd;
	string verb,args;

	verb = input;

	set_fail_reason(nil);

	/* pretty up input */
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

	write("(obj/actor cmd='" + cmd + "' args='" + args + "')");

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
