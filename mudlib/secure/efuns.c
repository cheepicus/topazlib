static object this_player()
{
	return this_user()->query_player();
}

static string str(mixed x)
{
	string s; int i; object o; mixed *k, *v;

	switch(typeof(x)) {
		case T_STRING: return x;
		case T_NIL: return "nil";
		case T_INT:
		case T_FLOAT: return x + "";
		case T_OBJECT: return "<" + object_name(x) + ">";
		case T_ARRAY: {
			if (!x || !(i=sizeof(x))) return "({ })";
			x += ({ });
			while (i--) x[i] = str(x[i]);
			return "({ " + implode(x,",") + " })";
		}
		case T_MAPPING: {
			if (!x) return "()";
			k = map_indices(x);
			if (!k || !(i=sizeof(k))) return "([ ])";
			k += ({ });
			while (i--) k[i] = str(k[i]) + "=" + str(x[k[i]]);
			return "([ " + implode(k,",") + " ])";
		}
	}
	return "?unknown?";
}


int sign(int x) { return x<0 ? -1 : (x>0 ? 1 : 0); }

static int index(mixed source, mixed to_find, varargs int delta)
{
	int i, j, c, sz, sz2, found;
	string s, *splode;
	if (!source || !to_find) return -1;
	if (!delta) delta = 1;

	/* array index */
	if (IsArray(source)) {
		sz = sizeof(source);
		if (!sz) return -1;
		if (delta > 0) {
			for(i=0; i<sz; i++) if (to_find == source[i]) return i;
		} else {
			for(i=sz; i--;) if (to_find == source[i]) return i;
		}
		return -1;
	}

	if (!IsString(source)) return -1;
	/* string index */
	if (IsInt(to_find)) {
		i = to_find; to_find = " "; to_find[0] = i;
	}
	if (!IsString(to_find)) return -1;
	if (source == "" || to_find == "") return -1;
	sz = strlen(source); sz2 = strlen(to_find);
	if (sz2 > sz) return -1;
	c = to_find[0];

	i = (delta>0) ? 0 : sz-1;
	for(; i>=0 && i<sz; i += delta) {
		if (source[i] != c) continue;
		for (j=0; 1; j++) {
			if (j >= sz2) return i;
			if (i+j >= sz) return -1;
			if (source[i+j] != to_find[j]) break;
		}
	}

	return -1;
}

static int write(string s, varargs int no_newline)
{
	object u;
	u = this_user();
	if (!u) return 0;
	u->accept_write(s, no_newline);
	return 1;
}

static int writef(string fmt, varargs string args...)
{
	object u; string *strs; int i,j;

	u = this_user();
	if (!u) return 0;
	fmt = "%%" + fmt;
	strs = explode(fmt,"%");
	for (i=0,j=2; i<sizeof(args) && j < sizeof(strs); i++,j+=2)
	{
		strs[j] = args[i];
	}
	u->accept_write(implode(strs,""));
}

static int starts_with(string test, string prefix)
{
	int sz_test, sz_pref;
	sz_test = strlen(test);
	sz_pref = strlen(prefix);
	if (sz_test < sz_pref) return 0;
	return test[..sz_pref-1] == prefix;
}

static int ends_with(string test, string suffix)
{
	int sz_test, sz_suff, i;
	sz_test = strlen(test);
	sz_suff = strlen(suffix);
	if (sz_test < sz_suff) return 0;
	return test[sz_test-sz_suff..] == suffix;
}

static string lower_case(string str)
{
	int i,c;
	if (!str || str=="") return str;
	for (i=strlen(str); i--;) {
		c = str[i];
		if (c < 'A' || c > 'Z') continue;
		str[i] |= 0x20;
	}
	return str;
}

static string capitalize(string str)
{
	int c;
	if (!str || str=="") return str;
	c = str[0];
	if (c >= 'a' && c <= 'z') {
		c = c - 'a' + 'A';
		str[0] = c;
	}
	return str;
}

static string an(mixed str)
{
	int c;
	if (IsObject(str)) return an(str->query_name());
	if (!str || !IsString(str) || str == "") return str;
	c = str[0];
	if (c>='a' && c<='z' && !starts_with(str,"the ")) {
		if (c=='a' || c=='e' || c=='i' || c=='o' || c=='u')
			str = "an " + str;
		else
			str = "a " + str;
	}
	return str;
}

static string the(mixed x)
{
	string str,s;
	str = an(x);
	if (!str || str=="") return str;
	if (sscanf(str,"a %s",s) || sscanf(str,"an %s",s))
		str = "the " + s;
	return str;
}

static string sprint(string str, varargs string args...)
{
	int i,sz,x;
	string *arr;
	i = (str[0] == '|') ? 0 : 1;
	arr = explode(str,"|");
	for (sz=sizeof(arr); i<sz; i+=2) {
		if (sscanf(arr[i],"%d",x) == 1) {
			arr[i] = args[x-1];
		}
	}
	return implode(arr,"");
}


static void driver_error(string s)
{
	Driver->driver_error(s);
}

static void error(string s)
{
	Driver->driver_error(s);
	::error(s);
}

static string query_ip_number(object u)
{
	string s;
	s = ::query_ip_number(u);
	if (s == "::1") return "127.0.0.1";
	return s;
}

/* this is probably garbage */
static object this_actor()
{
	object o;
	int i;

	for (i=0,o=this_object(); o; o = previous_object(i++)) {
		if (function_object("is_actor",o) == "/obj/actor") return o;
	}
	return nil;
}

int fail(string reason)
{
	object o;
	o = this_actor();
	o->set_fail_reason(reason);
	return 0;
}

void input_to(string func, varargs int noecho)
{
	this_user()->set_input_to(this_object(), func);
	if (noecho) this_user()->echo(0);
}

#define Spaces50 "                                                  "
string pad(string s, int width, int align)
{
	int len, x, padlen;
	string pad;

	len = strlen(s);
	pad = "";
	while ( (x=width-len-padlen) > 0) {
		x = Min(50, x);
		pad += Spaces50[..x-1];
		padlen = strlen(pad);
	}
	if (padlen) {
		if (align < 0) s = pad + s;
		else if (align > 0) s = s + pad;
		else {
			x = padlen/2;
			s = pad[..x-1] + s + pad[..padlen-x-1];
		}
	}
	return s;
}

static void dwrite(varargs mixed args...)
{
	int i;
	object u;
	string s;

	if(!(u = this_user())) return;
	if (!args || !(i=sizeof(args))) return;
	args += ({});
	while (i--) args[i] = str(args[i]);
	u -> accept_write(implode(args,""));
}

static string trim(string s)
{
	int b,e;
	if(!s || !(e=strlen(s))) return "";
	while (s[b]<=' ') b++;
	while (s[--e]<=' ');
	return s[b..e];
}

static string *explode_and_trim(string s, string sep)
{
	string *ret;
	int i;
	ret = explode(s, sep);
	for (i=sizeof(ret); i--; ) {
		ret[i] = trim(ret[i]);
	}
	return ret;
}

static string* grab_file(string f)
{
	string s;
	s = read_file(f);
	return explode(s,"\n");
}

