
private static string _owner;

#include "/secure/efuns.c"

nomask string base_name(varargs object o)
{
	string s,oname;

	if (!o) o = this_object();
	s = object_name(o);
	sscanf(s,"%s#%*d",s);
	return s;
}

nomask void set_owner(string newowner)
{
	/* LOG THIS */
	write("OWNER of " + object_name(this_object()) + " is now " + newowner);
	if (previous_program() != User) return;
	if (newowner == Secure) return;
	_owner = newowner;
}

nomask string owner()
{
	return _owner;
}

void create() { }

nomask void __create__()
{
	string *s;
	s = explode(object_name(this_object()), "/");
	if (s[0] == "wiz" && sizeof(s) > 1) {
		_owner = "/wiz/" + s[1];
	} else {
		_owner = "/" + s[0];
	}
	create();
}

static object find_user(string name)
{
	object *us,u;
	int i;
	if (!name || name == "") return nil;
	us = users();
	for (i=sizeof(us); i--; ) {
		u = us[i];
		if (!u) continue;
		if (u->query_busy() == "login") return nil;
		if (u->query_user_name() == name) {
			return u;
		}
	}
	return nil;
}

static object find_player(string name)
{
	return Driver->find_player(name);
}

static string normalize_path(string base, string rel, varargs string wiz)
{
	return Driver->normalize_path(base,rel,wiz);
}

static int valid_read(string path)
{
	return Driver->valid_read(this_object(),path);
}

static int valid_write(string path)
{
	return Driver->valid_write(this_object(),path);
}

static string read_file(string file, varargs int offset, int size)
{
	file = normalize_path(file,"");
	if (valid_read(file)) {
		return ::read_file(file, offset, size);
	}
	return nil;
}

static int write_file(string file, string str, varargs int offset)
{
	file = normalize_path(file,"");
	if (valid_write(file)) {
		return ::write_file(file, str, offset);
	}
	return 0;
}

static int remove_file(string file)
{
	file = normalize_path(file,"");
	if (valid_write(file)) {
		return ::remove_file(file);
	}
	return 0;
}

static int rename_file(string from, string to)
{
	from = normalize_path(from,"");
	to = normalize_path(to,"");
	if (valid_write(from) && valid_write(to)) {
		return ::rename_file(from,to);
	}
	return 0;
}

static int make_dir(string dir)
{
	dir = normalize_path(dir,"");
	if (valid_write(dir)) {
		return ::make_dir(dir);
	}
	return 0;
}

static int remove_dir(string dir)
{
	dir = normalize_path(dir,"");
	if (valid_write(dir)) {
		return ::remove_dir(dir);
	}
	return 0;
}

static mixed** get_dir(string path)
{
	string *arr,dir,file;
	int sz;
	path = normalize_path(path,"");
	arr = explode(path,"/");
	sz = sizeof(arr);
	file = arr[sz-1];
	arr[sz-1] = "";
	dir = "/" + implode(arr,"/");

	/* return nil, not empty arrays, so caller can tell not readable. */
	if (!valid_read(dir)) return nil;
/*
	write("valid_read of " + dir + " == " + valid_read(dir));
	write("get_dir path="+path+" dir="+dir+" file="+file);
*/
	return ::get_dir(path);
}

static int file_size(string file) {
	mixed **dir;
	int i;

	if (file == "/") return -2;
	dir = ::get_dir(file);
	if (sizeof(dir[0]) != 1) return -1;
	return dir[1][0];
}

static object clone_object(mixed o)
{
	string s;
	if (IsString(o)) o = find_object(o);
	if (!o) return nil;
	s = object_name(o);
	if (sscanf(s,"%*s.inh") || sscanf(s,"%*s.d"))
		error("Can't clone an inheritable or daemon.");
	return ::clone_object(o);
}

void destruct_object(object o)
{
	o->destruct();
}

void destruct()
{
	::destruct_object(this_object());
}
