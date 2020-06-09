
string ftime(int t)
{
	string s,m,hms;
	int y,d;
	sscanf(ctime(t),"%*s %s %d %s %d", m, d, hms, y);
	switch (m) {
		case "Jan": m = "01"; break;
		case "Feb": m = "02"; break;
		case "Mar": m = "03"; break;
		case "Apr": m = "04"; break;
		case "May": m = "05"; break;
		case "Jun": m = "06"; break;
		case "Jul": m = "07"; break;
		case "Aug": m = "08"; break;
		case "Sep": m = "09"; break;
		case "Oct": m = "10"; break;
		case "Nov": m = "11"; break;
		case "Dec": m = "12"; break;
	}
	return y + "-" + m + "-" + (d<10?"0":"") + d + " " + hms;
}

string d_str(mixed m)
{
	 string s; int i;
	 object o; mixed *arr;
	 if (IsNil(m)) return "nil";
	 if (IsString(m)) return m;
	 if (IsInt(m) || IsFloat(m)) return "" + m;
	 if (IsObject(m)) return "<" + object_name(m) + ">";
	 if (IsArray(m)) {
		 s = "({";
		 for (i=0; i<sizeof(m); i++) {
			 if (i) s += ",";
			 s += d_str(m[i]);
		 }
		 s += "})";
		 return s;
	 }
	 if (IsMapping(m)) {
		 string *k; mixed *v;
		 k = map_indices(m);
		 v = map_values(m);
		 s = "([";
		 for(i=0; i<sizeof(k); i++) {
			 if (i) s += ",";
			 s += "\"" + k[i] + "\":" + d_str(v[i]);
		 }
		 s += "])";
		 return s;
	 }
	 return "(Unknown)";
}

string pretty_stack()
{
	mixed **trace, *t;
	string s, spaces;
	int i, j;

	trace = call_trace();
	spaces = "";
	s = "";
	for (i = sizeof(trace) - 2; i--;) {
		t = trace[i];

		for (j=5; j<sizeof(t); j++) /* prettify args */
			t[j] = d_str(t[j]);

		spaces += " ";

		s += "\n\t" + i + spaces;
		s += "<" + t[0] + "> " + t[1] + ".c:" + t[3] + " " + t[2] + "("
				+ implode(t[5..],", ") + ")";

	}
	return s;
}

#define TIMES_S ({ 365*24*60*60, 30*24*60*60, 7*24*60*60, 24*60*60, 60*60, 60, 1 })
#define NAMES_S ({ "year", "month", "week", "day", "hour", "minute", "second" })

private int *times;
private string *names;

string elapsed(int seconds, varargs int full)
{
	string ret; int i,x;
	if (!times) { times = TIMES_S; names = NAMES_S; }
	ret = "";
	for (i=0; i < sizeof(times); i++) {
		if (seconds <= times[i]) continue;
		if (ret != "") ret += " ";
		x = seconds / times[i];
		ret += x + " " + names[i];
		if (x != 1) ret += "s";
		if (!full) return ret;
		seconds -= x * times[i];
	}
	return ret;
}

void msg(string s)
{
	send_message(ftime(time()) + " " + s + "\n");
}

void msgs(varargs string ss...)
{
	int i;
	send_message(ftime(time()) + " " + implode(ss," ") + "\n");
}

void driver_error(string s)
{
	msg("ERROR: " + s);
	msg(pretty_stack());
}

static void load(string s)
{
	msg("Loading " + s);
	compile_object(s);
}

/* Called when the system starts up after a reboot. */
static void initialize()
{
	msg("____________________________________");
	msg("DGD initalize()");
	load(Auto);
	load(User);
	load(Player);
	load(WizSoul);
	load(Nowhere);

}

string normalize_path(string base, string rel, varargs string wiz)
{
	string path, *in_arr, *out_arr, s;
	int i,j,sz;

	if (!rel || rel == "") return base;
	if (rel[0] == '/') {
		path = rel;
	} else {
		path = base + "/" + rel;
	}

	in_arr = explode(path,"/");
	sz = sizeof(in_arr);
	out_arr = allocate(sz);
	for (i=0,j=0; i<sz; i++) {
		s = in_arr[i];
		if (s=="" || s==".") continue;
		if (s=="..") {
			if(j>0) j--;
			continue;
		}
		if (s[0]=='~' && wiz) {
			out_arr[0] = "wiz"; /* should be smarter */
			out_arr[1] = (s=="~") ? wiz : s[1..];
			j = 2;
			continue;
		}
		out_arr[j] = in_arr[i];
		j++;
	}
	return "/" + implode(out_arr[..j-1],"/");
}

int valid_read(object o, string path)
{
	string owner;
	if(!o) return 0;
	owner = o->owner();
	if (sscanf(path,"%*s..%*s") || sscanf(path,"%s~%s")) return 0;
	if (owner == "/secure") return 1;
	if (sscanf(path,"/secure/%*s")) return 0;
	if (sscanf(path,"/include/%*s")) return 1;
	if (sscanf(path,"/doc/%*s")) return 1;
	if (sscanf(path,"/log/%*s")) return 1;
	if (!owner) return 0;
	return 1;
}

int valid_write(object o, string path)
{
	string owner;
	if  (!o) return 0;
	owner = o->owner();
	if (sscanf(path,"%*s..%*s") || sscanf(path,"%s~%s")) return 0;
	if (sscanf(path,"/secure%*s")) return 0;
	if (path == "/") return 0;
	if (owner == "/secure") return 1;
	if (sscanf(path,owner + "/%*s")) return 1;
	return 0;
}


/*
	Handle a compile-time error; there can be several for the same
	LPC file before a runtime error results (calling runtime_error).
*/
static void compile_error(string file, int line, string err)
{
	object u; string s;
	s = "** Compile Error: " + file + ":" + line + " " + err;
	msg(s);
	if (u = this_user()) u->accept_write(AnsiFGRed + s + AnsiReset);
}

/*
	Called when a runtime error occurs.  <caught> is 0 if the error
	is not caught, or 1 + an index in the return value of call_trace(),
	indicating the frame in which the error is caught.  runtime_error()
	is called with rlimits (-1; -1) so it will not run out of stack or
	ticks.  <ticks> specifies the amount of ticks left at the moment
	the error occurred.  The return value is the new error string.
*/
static string runtime_error(string error, int caught, int ticks)
{
	string s;
	object u;
	s = "** Runtime Error: " + error + pretty_stack();
	msg(s);
	if (u = this_user()) u->accept_write(AnsiFGRed + s + AnsiReset);
	return s;
}

/* look for (and possibly load) player object */

object find_player(string name)
{
	string path,s;
	object o;

	path = "/P/" + name;
	o = find_object(path);
	if (o) return o;
	s = read_file(path + ".sav",0,1);
	if (!s) return nil;
	s = read_file("/obj/player.c");
	o = compile_object(path,s);
	return o;
}

/*
	Get the object named by objname as the first argument of a call_other().
	compile_object() may be called from here.
*/
static object call_object(string opath)
{
	object o;
	string p;
	if (sscanf(opath,"%*s.inh")) return nil;
	o = find_object(opath);
	if (o) return o;
	if (sscanf(opath,"/P/%s",p) == 1) {
		return find_player(p);
	}
	return compile_object(opath);
}

/*
	Called if the driver receives a kill signal.  The function should
	shut down the system, and possibly dump state.
*/
static void interrupt()
{
	object *users; int i;
	users = users();
	for (i=sizeof(users); i--;) {
		users[i]->quit();
	}
}

/*
	Return an object for a new connection on the given telnet port.
*/
static object telnet_connect(int port)
{
	object o;
	o = find_object(User);
	if (!o)
	{
		o = compile_object(User);
		o = clone_object(o);
	}
	return clone_object(o);
}

/*
	Return an object for a new connection on the given binary port.
*/
static object binary_connect(int port)
{
	object o;
	o = telnet_connect(port);
	o -> set_binary(1);
	return o;
}

/*
	Called whenever the program of a master object is removed (because
	the object has been destructed, and all clones and inheriting objects
	have also been destructed).  This function is called with
	rlimits (-1; -1).
	The "index" is a unique number for each master object, also
	available with status(obj), which can help distinguishing between
	different issues of the same object.
*/
static void remove_program(string objname, int timestamp, int index) {}


/*
	Used to translate a path for an editor read command.  If nil is
	returned, the file cannot be read.
*/
static string path_read(string ed_path)
{
	string dir, wiz;
	if (!previous_object()) return nil;
	dir = previous_object()->query_ed_dir();
	if (!dir) dir = "/";
	sscanf(previous_object()->owner(),WizHome("%s"),wiz);
	ed_path = normalize_path(dir,ed_path,wiz);
	if (valid_read(previous_object(),ed_path)) return ed_path;
	return nil;
}

/*
	Used to translate a path for an editor write command.  If nil is
	returned, the file cannot be written to.
*/
static string path_write(string ed_path)
{
	string dir, wiz;
	if (!previous_object()) return nil;
	dir = previous_object()->query_ed_dir();
	if (!dir) dir = "/";
	sscanf(previous_object()->owner(),WizHome("%s"),wiz);
	ed_path = normalize_path(dir,ed_path,wiz);
	if (valid_write(previous_object(),ed_path)) return ed_path;
	return nil;
}


/*
	Called after the system has restarted from a snapshot.  The argument
	will be 1 if connections and editor sessions were restored as well.
*/
static void restored(varargs int hotboot) {}


/*
	Called just before a function is called in an object which has been
	marked by call_touch().  A non-zero return value indicates that the
	object's "untouched" status should be preserved through the following
	call.
*/
static int touch(object o, string func) {}

/*
	Get the object named by program as an object to inherit.  The third
	argument is 1 for private inheritance, 0 otherwise.
	compile_object() may be called from here.
*/
static object inherit_program(string file, string program, int priv)
{
	object o;
	if (sscanf(program,"%*s.d")) return nil;
	o = find_object(program);
	if (!o) o = compile_object(program);
	return o;
}

/*
	Used to translate include path <path> when included from the file
	<file>.  The return value can be either string for the translated path,
	in which case an attempt is made to read the included file from that
	location, or an array of strings, the concatenation of which represents
	the included file itself.  If any other value is returned, the file
	<path> cannot be included.
*/
static mixed include_file(string file, string path)
{
	msg("include_file " + file + "," + path);
}

/*
	If an object A is loaded, A inherits B, B inherits C, and the
	version of C inherited by B is out of date, recompile(B) will be
	called in the driver object.  If B should actually be recompiled
	(inheriting the new version of C from B), the driver object must
	destruct B; if this is done, A will inherit the most recent
	versions of B and C.
*/
static void recompile(object obj)
{
	destruct_object(obj);
}


/*
	Called when a runtime error occurs in atomic code.  <atom> is an
	an index in the return value of call_trace(), indicating the frame in
	which the atomically executed code starts.  atomic_error() is called
	with rlimits (-1; -1) so it will not run out of stack or ticks.
	<ticks> specifies the amount of ticks left at the moment the error
	occurred.  The return value is the new error string.
*/
static string atomic_error(string error, int atom, int ticks)
{
	return error + " atom=" + atom + " ticks=" + ticks;
}

/*
	Called when <file> is compiled, to translate the object path <type>
	used in a type declaration, cast or <- operator.
*/
static string object_type(string file, string type) { return nil; }

/*
	An object uses the rlimits (stackdepth; ticks) { ... }
	construct, which can be used to assign arbitrary limits to function
	call stack depth and ticks; running out of ticks during execution
	causes an error.  -1 signifies an infinite amount of stack space or
	ticks.
	During compilation, driver->compile_rlimits(objname) is called.  If
	non-zero is returned, the object will be able to use rlimits at
	runtime with no restrictions.  If 0 is returned,
	driver->runtime_rlimits() will be called at runtime to further
	verify if an object is allowed to change its rlimits (resource
	limits).
*/
static int compile_rlimits(string objname)
{
	return 0;
}

/*

	Called at runtime for rlimits constructs not already cleared by
	compile_rlimits().  If 0 is returned, the rlimits usage is illegal
	and is aborted with an error.
	Values for stack and ticks: < 0: infinite, 0: no change,
	> 0: set to specified amount.
*/
static int runtime_rlimits(object obj, int stack, int ticks)
{
	return 0;
}
