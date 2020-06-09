
inherit "/obj/base.inh";

mapping exits;

void add_exit(string dir, string target)
{
	if (!dir || dir=="") return;
	if (!exits) exits = ([]);
	exits[dir] = target;
}

mapping query_exits() { return exits; }

object query_exit(string dir)
{
	string s; object o;
	if (!exits) return nil;
	s = exits[dir];
	if (!s) return nil;
	o = find_object(s);
	if (!o) {
		s->loadloadload();
		o = find_object(s);
	}
	return o;
}

/* pretty list of exits */
string exits()
{
	string *keys, *vals, ret;
	int i, sz;

	if (!exits) return nil;
	keys = map_indices(exits);
	if (!keys || !(sz=sizeof(keys))) return nil;
	for (i=0; i<sz; i++) keys[i] = AnsiBold + keys[i] + AnsiReset;

	if (sz == 1) return keys[0];
	if (sz == 2) return implode(keys," and ");
	keys[sz-1] = "and " + keys[sz-1];
	return implode(keys, ", ");
}
