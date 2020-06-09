/* These are "low security" wizard commands which wizards
 * can use to help manage mud objects and their files.
 */

private static string wizname;
private static string currdir;
private static string it_objname;

void create()
{
}

string query_ed_dir() { return currdir; }

/* Produce a line of text */
string objdesc(object o, varargs int nopad)
{
	string s;
	nopad = !nopad;
	if (!o) return "nil";
	s = pad(str(o->short()),nopad*16,-1) + " " + pad(str(o),nopad*16,1);
	s += pad(" lt:" + str(o->query_light()) + "/" + str(o->query_total_light()),nopad*9,1);
	s += pad(" wt:" + str(o->query_weight()) + "/" + str(o->query_total_weight()),nopad*9,1);
	return s;
}



object set_wizname(string name)
{
	if (previous_program() == User) {
		wizname = name;
		currdir = WizHome(wizname);
	}
}

static int cmd_cd(string dir)
{
	if (!dir || dir == "") dir = "~";
	dir = normalize_path(currdir, dir, wizname);
	if (file_size(dir) != -2) {
		dwrite(dir, ": not a directory.");
	} else {
		currdir = dir;
	}
	return 1;
}

static int cmd_dir(string str)
{
	mixed **dir;
	str = normalize_path(currdir,str,wizname);
	dir = get_dir(str);
	if (!dir) {
		dwrite("Couldn't get_dir for ", str);
	}
	else {
		dwrite(dir);
	}
	return 1;
}

static int cmd_pwd(string ignored)
{
	dwrite(currdir);
	return 1;
}

#define SPACES16 "                "
static int cmd_ls(string args)
{
	mixed **info;
	int i,sz,tabs,linelen;
	string s,dir,out;
	int cols, len;

	args = normalize_path(currdir, args, wizname);
	dir = args;
	if (file_size(dir) == -1) {
		dwrite(args," does not exist.");
	}
	if (file_size(dir) == -2) dir = dir + "/*";
	if (dir == "//*") dir = "/*";
	info = get_dir(dir);
	if (!info) {
		dwrite(args, " is not readable.");
		return 1;
	}
	out = ""; linelen = 0;
	sz = sizeof(info[0]);

	if (sz == 0) {
		dwrite(args, " has no files.");
	}

	cols = this_user()->query_columns();

	for (i=0; i<sz; i++) {
		s = info[0][i];
		len = strlen(s);
		if (info[1][i] == -2) {
			s = AnsiFGBlue + s + "/" + AnsiReset;
			len++;
		} else if ((ends_with(s,".c") || ends_with(s,".o"))
				&& find_object(args + "/" + s[..len-3])) {
			s = AnsiFGCyan + s + "*" + AnsiReset;
			len++;
		}

		if (linelen + len > cols - 33) {
			out += s;
			if (i == sizeof(info[0])-1) break;
			out += "\n";
			linelen = 0;
		} else {
			int numspace;
			numspace = 16 - (len%16);
			s += SPACES16[..numspace-1];
			linelen += len + numspace;
			out += s;
		}
	}
	dwrite(out);
	return 1;
}

int cmd_ed(string path)
{
	path = normalize_path(currdir,path,wizname);
	dwrite("Editing ", path);
	dwrite(trim(editor("e " + path) + editor("1 z #")));
	return 1;
}

int cmd_cat(string path)
{
	path = normalize_path(currdir, path, wizname);
	dwrite(read_file(path));
	return 1;
}

int cmd_rw(string path)
{
	path = normalize_path(currdir, path, wizname);
	dwrite("valid_read(", path, ") == ", valid_read(path));
	dwrite("valid_write(", path, ") == ", valid_write(path));
	return 1;
}

int cmd_rmdir(string path)
{
	path = normalize_path(currdir, path, wizname);
	if(!remove_dir(path))
		dwrite("Could not remove directory ", path);
	return 1;
}

int cmd_mkdir(string path)
{
	path = normalize_path(currdir, path, wizname);
	if(!make_dir(path))
		dwrite("Could not make directory ", path);
	return 1;
}

int cmd_rm(string file)
{
	file = normalize_path(currdir, file, wizname);
	if(!remove_file(file))
		dwrite("Could not remove ", file);
	return 1;
}

int cmd_mv(string args)
{
	string from,to;
	if (sscanf(args,"%s %s",from,to) != 2) {
		dwrite("Usage: mv <from_file> <to_file>");
		return 1;
	}
	from = normalize_path(currdir, from, wizname);
	to = normalize_path(currdir, to, wizname);
	if(!rename_file(from,to))
		dwrite("Could not rename ", from, " to ", to);
	return 1;
}

int cmd_cp(string args)
{
	string from, to, chunk;
	int numread;
	if (sscanf(args,"%s %s",from,to) != 2) {
		dwrite("Usage: cp <from_file> <to_file>");
		return 1;
	}
	from = normalize_path(currdir,from,wizname);
	to = normalize_path(currdir,to,wizname);
	if (file_size(to) != 0) {
		dwrite(to, " already exists");
		return 1;
	}
	/* let's use ed, the standard text editor */
	dwrite(trim(editor("e " + from)));
	dwrite(trim(editor("w " + to)));
	dwrite(trim(editor("q!")));
	return 1;
}

int cmd_boom(string args)
{
	dump_state();
	return 1;
}

int cmd_eval(string args)
{
	mixed ret;
	string s, f;
	object o;
	f = WizHome(wizname) + "/eval_temp";
	s = "#define me (this_user()->query_player())\n"
		+ "#define env(o) ((o)->environment())\n"
		+ "string s,t,u,v; object o,p,q,r; int i,j,k,x,y,z;\n"
		+ "mixed eval() { return ("+args+"); }";
	remove_file(f + ".c");
	if(write_file(f + ".c", s)) {
		catch { ret = f->eval(); };
		dwrite(" ==> ", ret);
		remove_file(f + ".c");
		o = find_object(f);
		if (o) o->destruct();
	}
	return 1;
}



object wizfind_object(string path, varargs int dont_load)
{
	string *arr, s, tmp;
	int i,x;
	object o, *inv, otmp;

	arr = explode(path,":");
	for (i=0; i<sizeof(arr); i++) {
		s = arr[i];
		if (s == "me") { o = find_player(wizname); continue; }
		if (sscanf(s,"@%s",tmp)) { o = find_player(tmp); continue; }
		if (s == "env") {
			if (!o) o = find_player(wizname);
			otmp = o->environment();
			if (!otmp) { dwrite("No environment of ",o); return nil; }
			o = otmp;
			continue;
		}
		if (s == "it") {
			o = find_object(it_objname);
			if (!o && !dont_load) call_other(it_objname,"load_gosh_darnit");
			if (o) continue;
			dwrite(it_objname," doesn't exist, sorry.");
			it_objname = nil;
			return nil;
		}
		if (sscanf(s,"%d",x)) {
			if (!o) o = find_player(wizname);
			inv = o->all_inventory();
			if (!inv || x >= sizeof(inv)) {
				dwrite("No inventory #", x, " in ", o);
				return nil;
			}
			o = inv[x];
			continue;
		}
		tmp = normalize_path(currdir,s,wizname);
		if (find_object(tmp)) { o = find_object(tmp); continue; }
		if (dont_load) { dwrite("Couldn't find " + tmp); return nil; }
		if (file_size(tmp + ".c") > 0) {
			dwrite("Loading " + tmp);
			tmp->loadloadload();
			o = find_object(tmp);
			continue;
		}
		dwrite("Couldn't find " + path);
		return nil;
	}
	return o;
}

static int cmd_it(string str)
{
	object o;
	if (!str || str=="") {
		if (!it_objname) {
			dwrite("No 'it' remembered.");
			return 1;
		}
		o = find_object(it_objname);
	} else {
		if (!(o = wizfind_object(str))) {
			call_other(str,"???");
			o = find_object(str);
		}
		it_objname = object_name(o);
	}
	dwrite("it = " + it_objname + " object:" + objdesc(o,1));
	return 1;
}

static int cmd_call(string str)
{
	mixed *arr,ret;
	string func,s,argstr;
	object o,otmp;
	int i, itmp;
	float ftmp;

	arr = explode(str," ");
	if (sizeof(arr) < 2) return 1;

	o = wizfind_object(arr[0]);
	if (!o) return 1;
	func = arr[1];

	arr = arr[2..];
	argstr = implode(arr,", ");
	for (i=sizeof(arr); i--;) {
		if (!arr[i]) continue;
		otmp = find_object(arr[i]);
		if (otmp) { arr[i] = otmp; continue; }
		if (index(arr[i],':') >= 0) {
			otmp = wizfind_object(arr[i]);
			if (otmp) { arr[i] = otmp; continue; }
		}
		if (sscanf(arr[i],"\"%s\"",arr[i])) continue;
		if (arr[i][0] == 'e') continue; /* sscanf misreads as float! */
		if (sscanf(arr[i],"%d",itmp)) { arr[i] = itmp; continue; }
		if (sscanf(arr[i],"%f",ftmp)) { arr[i] = ftmp; continue; }
	}

	dwrite("Calling ",o,"->",func,"(",argstr,") defined in ",function_object(func,o));

	switch (sizeof(arr)) {
		case 0: ret = call_other(o,func); break;
		case 1: ret = call_other(o,func,arr[0]); break;
		case 2: ret = call_other(o,func,arr[0],arr[1]); break;
		case 3: ret = call_other(o,func,arr[0],arr[1],arr[2]); break;
		case 4: ret = call_other(o,func,arr[0],arr[1],arr[2],arr[3]); break;
		default: ret = call_other(o,func,arr[0],arr[1],arr[2],arr[3],arr[4]); break;
	}
	dwrite(" ==> ",ret);
	return 1;
}

int cmd_update(string path)
{
	object o;
	path = normalize_path(currdir,path,wizname);
	o = find_object(path);
	if (o)
	{
		dwrite("Updating ", path);
		compile_object(path);
		dwrite("Ok.");
	} else {
		dwrite("Object ", path, " not loaded.");
	}
	return 1;
}

int cmd_I(string args)
{
	object o, *inv;
	int i;
	if (!args || args=="") args = "me";
	o = wizfind_object(args,1);
	if (!o) return 1;
	dwrite("--- ",objdesc(o,1));
	inv = o->all_inventory();
	if (!inv) {
		dwrite("(Empty)");
	} else {
		for (i=0; i<sizeof(inv); i++) {
			dwrite(pad(i+"",3,-1), " ", objdesc(inv[i]));
		}
	}
	return 1;
}

int cmd_G(string args)
{
	object o, me, tmp;
	me = find_player(wizname);
	o = wizfind_object(args);
	if (!o) return 1;
	dwrite("Going to ",o);
	me->__move(o);
	dwrite("Now at ", me->environment());
	return 1;
}

int cmd_T(string args)
{
	string str1, str2;
	object what, where;
	if (!args || args=="") { dwrite("T <object> [place]."); return 1; }
	sscanf(args, "%s %s", str1, str2);
	if (str2) {
		what = wizfind_object(str1);
		where = wizfind_object(str2);
	} else {
		what = wizfind_object(args);
		where = find_player(wizname)->environment();
	}
	if (!what) { dwrite("T what?"); return 1; }
	if (!where) { dwrite("T ", object_name(what), " where?"); return 1; }
	what->__move(where);
	return 1;
}

int cmd_D(string args)
{
	object o, *inv;
	int i,all;

	if (!args) {
		dwrite("Usage: D <object>\nDestructs object.");
		return 1;
	}
	if (args=="*") {
		all = 1;
		o = find_player(wizname)->environment();
		if (!o) { dwrite("You're not anywhere."); return 1; }
	} else if (ends_with(args,":*")) {
		all = 1;
		o = wizfind_object(args[..strlen(args)-3], 1);
	} else {
		o = wizfind_object(args, 1);
	}

	if (!o) {
		dwrite("Couldn't find " + args);
		return 1;
	}

	if (all) inv = o->all_inventory();
	else inv = ({ o });

	for (i=sizeof(inv); i--;) {
		if (starts_with(object_name(inv[i]),"/P/")) {
			dwrite("Skipping ", inv[i]);
			continue;
		}
		dwrite("Destructing ", inv[i]);
		if(inv[i]) inv[i]->destruct();
	}
	dwrite("Done.");
	return 1;
}

#define ST_NAMES ({ "version", "start time", "boot time", "uptime", "swap size",\
 	"swap used", "sector size", "swapped 1m", "swapped 5m",\
	"static mem alloced", "static mem used", "dynamic mem alloced", "dynamic mem used",\
	"object table size", "objects", "callout table size", "normal callouts", "long callouts",\
	"user table size", "editor table size", "max str size", "max arr/map size",\
 	"rem stack", "rem ticks", "precompiled objs", "telnet ports", "binary ports" })

#define O_NAMES ({ "compile time", "prog size", "data size", "sectors", "callouts", "master?", "undefined funcs" })

int cmd_stat(string args)
{
	mixed *arr;
	object o;
	int i, sz;
	string sep;

	if (!args || args=="") {
		string *stnames;
		stnames = ST_NAMES;
		arr = status();
		for (i=0, sz=sizeof(arr); i<sz; i++) {
			if (i==1 || i==2) arr[i] = Driver->ftime(arr[i]);
			if (i==3) arr[i] = Driver->elapsed(arr[i],1);
			dwrite(pad(stnames[i],20,-1), "  "+AnsiFGYellow, str(arr[i]), AnsiReset);
		}
	} else {
		string *onames;
		onames = O_NAMES;
		o = wizfind_object(args);
		dwrite("status of ",o);
		arr = status(o);
		for (i=0, sz=sizeof(arr); i<sz; i++) {
			if (i==0) arr[i] = Driver->ftime(arr[i]);
			dwrite(pad(onames[i],20,-1)," "+AnsiFGYellow,str(arr[i]), AnsiReset);
		}
	}
	return 1;
}


int handle_wizard_command(string str)
{
	string cmd,args;
	int ret;

	if (previous_program() != User) return 0;
	if (previous_object()->owner() != Secure) return 0;
	if (this_user()->query_user_name() != wizname) return 0;

	if (query_editor(this_object())) {
		string s;
		s = editor(str);
		if (s) write(s,1);
		return 1;
	}

	if (sscanf(str,"%s %s",cmd,args) != 2) {
		cmd = str;
		args = "";
	}

	switch (cmd) {

		/* file commands */
		case "cd": ret = cmd_cd(args); break;
		case "dir": ret = cmd_dir(args); break;
		case "pwd": ret = cmd_pwd(args); break;
		case "ls": ret = cmd_ls(args); break;
		case "ed": ret = cmd_ed(args); break;
		case "cat": ret = cmd_cat(args); break;
		case "rw": ret = cmd_rw(args); break;
		case "rmdir": ret = cmd_rmdir(args); break;
		case "mkdir": ret = cmd_mkdir(args); break;
		case "rm": ret = cmd_rm(args); break;
		case "mv": ret = cmd_mv(args); break;
		case "cp": ret = cmd_cp(args); break;

		/* object commands */
		case "it": ret = cmd_it(args); break;
		case "call": ret = cmd_call(args); break;
		case "boom": ret = cmd_boom(args); break;
		case "eval": ret = cmd_eval(args); break;
		case "update": ret = cmd_update(args); break;
		case "I": ret = cmd_I(args); break;
		case "G": ret = cmd_G(args); break;
		case "T": ret = cmd_T(args); break;
		case "D": ret = cmd_D(args); break;

		case "stat" : ret = cmd_stat(args); break;
	}

	return ret;
}
