/*
	A minimal object that can appear in the game.
	Can contain other things and be looked at.
*/

string name; /* most basic ID, properly capitalized, used for short() */
mapping ids; /* addtional things we identify too */
static string short_desc;
static string long_desc;

private static object *inv;
private static object env;
int my_light, my_weight;
static int inv_light, inv_weight;

/* flags */
int invisible;
int container;
int open;
int opaque;

int is_visible() { return !invisible; }
void set_visible(int v) { invisible = !v; }

int is_container() { return container; }
void set_container(int c) { container = !!c; }

int is_opaque() { return !env || opaque; }
void set_opaque(int opq)
{
	if (!!opq == is_opaque()) return;
	opaque = !!opq;
	if (inv_light && env)
		env->__add_inv_light( opq ? -inv_light : inv_light);
}

int is_open() { return open; }
void set_open(int op)
{
	if (!is_container()) return;
	open = !!op;
	if (is_opaque() && inv_light && env)
		env->__add_inv_light( op ? inv_light : -inv_light);
}

/* set_name nukes all the other ids */
void set_name(string str)
{
	if (!str || str=="") return;
	name = str;
	if (!ids) ids = ([]);
	ids[name] = 1;
	ids[lower_case(name)] = 1;
	short_desc = nil; /* this means refresh on next short() */
}

string query_name() { return name; }

/*
	Add a bunch of ids at once. If the name has not been set
	then the first argument will be used as the name
*/
void add_ids(varargs string n...)
{
	int i; string s;
	if (!n || !(i=sizeof(n))) return;
	if (!ids) ids = ([]);
	while (i--) {
		ids[n[i]] = 1;
		if (i==0 && !name) set_name(n[i]);
	}
}

mapping query_ids() { return ids; }

int id(string text)
{
	int i;
	if (invisible && !ids) return 0;
	/* visible things need to have an id */
	if (!ids) ids = ([ "strangeness":1 ]);
	return ids[text] || text == short_desc;
}

/*
	short() description is a word or two description of an object.
	Aim for ten characters or less. For named things, it should
	be just their name. For items, it should be no more than a
	noun plus an adjective.
*/
string calc_short() {
	if (!name || name=="") set_name("strangeness");
	return an(name);
}
string short()
{
	if (!short_desc) short_desc = calc_short();
	return short_desc;
}

/*
	A brief() description is longer than a short() but shorter than a
	long(). Think of it as being a concise description that is about
	half a line long, with some useful information. For example, a
	room's short() is just the name of the room, while the brief()
	includes exit information. For items it can include some tags.
*/
string brief()
{
	return short();
}

/* Multi-line description of an object itself. */
string calc_long()
{
	return short();
}
string long()
{
	if (!long_desc) long_desc = calc_long();
	return long_desc;
}


/* sets my personal light level */
void set_light(int newlight)
{
	int diff;
	diff = newlight - my_light;
	my_light = newlight;
	if (diff && env) env->__add_inv_light(diff);
}
void set_weight(int newweight)
{
	int diff;
	diff = newweight - my_weight;
	my_weight = newweight;
	if (diff && env) env->__add_inv_weight(diff);
}

/*
	Light shining upon an object from "outside". Includes light
	from inside the object if it's open or not opaque.

	A little complex: e.g. a item inside an open bag inside a
	glass box should receive light from the room the box is in.

*/
int query_light_upon()
{
	int lt, x;
	object e;
	lt = my_light;
	if (!env || open || !opaque) lt += inv_light;
	for (e=env; e; e=e->environment()) {
		x = e->query_total_light();
		if (x > lt) lt = x;
		if (e->is_opaque()) break;
	}
	return lt;
}

int query_light_within()
{
	if (!env || (opaque && !open)) return my_light + inv_light;
	else return query_light_upon();
}

int query_light() { return my_light; }
int query_total_light() { return my_light + inv_light; }
int query_weight() { return my_weight; }
int query_total_weight() { return my_weight + inv_weight; }
nomask object environment() { return env; }
nomask object *all_inventory() { return inv ? inv[..] : nil; }

nomask int within(object o) {
	object tmp;
	for(tmp = this_object(); tmp; tmp = tmp->environment()) {
		if (tmp == o) return 1;
	}
	return 0;
}

/* called when a lit object leaves or enters our inventory */
/* amt is light source of object. */
nomask void __add_inv_light(int amt)
{
	if (!amt) return;
	inv_light += amt;
	if (env && amt && (open || !opaque))
		env->__add_inv_light(amt);
}

nomask void __add_inv_weight(int amt)
{
	if (!amt) return;
	inv_weight += amt;
	if (env) env->__add_inv_weight(amt);
}


nomask int __add_inventory(object o, int olight, int oweight)
{
	int i;
	if (!o) return 0;
	if (!inv) inv = ({ o });
	else inv |= ({ o });
	__add_inv_light(olight);
	__add_inv_weight(oweight);
	return 1;
}

nomask int __remove_inventory(object o, int olight, int oweight)
{
	object *po;
	if (!inv || !o) return 0;
	po = ({ o });
	if (sizeof(inv & po)) {
		inv -= po;
		__add_inv_light(-olight);
		__add_inv_weight(-oweight);
		return 1;
	}
	return 0;
}

/* okay to call with destination nil: means remove, basically */
/* SHOULD NOT FAIL, so only call_other nomask functions in this file */
nomask void __move(object dest)
{
	int wt, lt, x;
	if (dest && dest->within(this_object())) {
		driver_error("destination within this_object()");
		return; /* only reason to fail?? */
	}
	wt = my_weight + inv_weight;
	lt = my_light;
	if (open || !opaque) lt += inv_light;
	if (env) {
		env->__remove_inventory(this_object(), lt, wt);
		env = nil;
	}
	if (!dest) return;
	dest->__add_inventory(this_object(), lt, wt);
	env = dest;
}

/* DO NOT CALL unless you know what you are doing */
/* this messes up inventory, weights, light */
nomask void __destruct_rec()
{
	if (inv)
	{
		int i; object *objs;
		objs = inv[..];
		for (i=sizeof(objs); i--;)
		{
			objs[i]->__destruct_rec();
		}
	}
	::destruct();
}

nomask void destruct()
{
	/* by default we destroy our inventory.  to save some light and
	 * weight correction, we're going to destroy our inventory without
	 * updating light and weight.  so we move this object to nowhere
	 * first so there's nothing really to update.
	 */
	__move(nil);
	__destruct_rec();
}


int move(object deest)
{
	object env = environment();
	if (env)
	{

	}

}

