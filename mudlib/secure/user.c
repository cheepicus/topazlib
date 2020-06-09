
string user_name;
string password;
string last_ip;
int last_on;

private static int last_input;
private static string prompt;
private static object player;
private static int savable_user;
private static int no_echo;
private static int columns;
private static int quitting;

private static object wizsoul;

private static int login_state;
#define LoginDone            0
#define LoginWantName        1
#define LoginCheckPassword   2
#define LoginNewPassword     3
#define LoginNewPassword2    4

string query_user_name() {
	if (login_state != LoginDone) return "";
	return user_name;
}

int query_last_input_time() {
	return last_input;
}

string busy_reason() {
	if (login_state != LoginDone) return "login";
	return nil;
}

void accept_write(string s, varargs int no_nl) {
	send_message(s);
	if (!no_nl) send_message("\n");
}

object query_player() {
	return player;
}

void echo(int echo) {
	send_message(echo);
	no_echo = !echo;
}

/*****
 *****  Login Stuff
 *****/

 static private int binary;
 void set_binary(int b) { binary = b; }

 private static string do_crypt(string str)
 {
	 string out;
	 int i;

	 str = hash_string("SHA1", str);
	 out = "";
	 for (i=0;i<strlen(str);i++) {
		 out += " ";
		 out[i] = ' ' + (str[i] % 97);
	 }
	 return out;
 }

 /* While this could be broken out into a separate
  * object or demon, that just confuses things.
  * Proper login is core security, don't get fancy.
  */

private static string clean_name(string str)
{
	int len, c, i;
	string *swears;

	len = strlen(str);
	if (len > 10 || len < 3) return "";
	while (len--) {
		c = str[len];
		if (c < 'A' || c > 'z' || (c > 'Z' && c < 'a')) {
			return "";
		}
		if (c < 'a') str[len] = c - 'A' + 'a';
	}

	swears = explode("fuck/shit/cunt/twat/dick/cock/tit/asshole/nigger/hitler/kike/jew","/");
	for (i = sizeof(swears); i--; ) {
		if (sscanf(str,"%*s" + swears[i] + "%*s")) return "";
	}

	return str;
}



private static void handle_login(string str)
{
	string s;
	object o;

	if (login_state == LoginWantName)
	{
		Driver->msgs(">> USER",str,query_ip_number(this_object()));
		str = clean_name(str);
		if (str == "") {
			write("Names should be 3-10 letters.\nWhat's your name? ",1);
			return;
		}
		user_name = str;
		if (restore_object(UserDataDir + user_name + ".sav")) {
			prompt = "What is your password? ";
			login_state = LoginCheckPassword;
		} else {
			write("New player!");
			prompt = "Enter a password (6+ characters): ";
			login_state = LoginNewPassword;
		}
		write(prompt,1);
		echo(0);
	}
	else if (login_state == LoginNewPassword) {
		if (strlen(str) < 6) {
			write("\nInvalid password.");
			destruct();
			return;
		}
		password = do_crypt(user_name + str);
		write("\nAgain: ",1);
		login_state = LoginNewPassword2;
		echo(0);
	}
	else if (login_state == LoginNewPassword2) {
		if (do_crypt(user_name + str) != password) {
			write("\nYou changed!\n");
			destruct();
			return;
		}
		Driver->msgs(">> NEW",user_name,query_ip_number(this_object()));
		write("\nWelcome, " + capitalize(user_name) + ".");
		login_state = LoginDone;
	}
	else if (login_state == LoginCheckPassword) {
		if (do_crypt(user_name + str) != password) {
			write("\nIncorrect password.\n");
			destruct();
			return;
		}
		if (o = find_user(user_name)) {
			Driver->msgs(">> RECONNECT",user_name,query_ip_number(this_object()));
			write("\nReplacing existing session.");
			player = o->query_player();
			player->set_user(this_object());
			o->quit(1);
		}
		Driver->msgs(">> LOGIN",user_name,query_ip_number(this_object()));
		write("\nWelcome back, " + capitalize(user_name) + ".");
		write("Your last login was from " + last_ip + ", ",1);
		write(Driver->elapsed(time()-last_on,1) + " ago.");
		login_state = LoginDone;
	}
}

void create() {

	if (object_name(previous_object()) == Driver) {
		login_state = LoginWantName;
		last_input = time();
		return;
	}
	driver_error("Illegal call to create() by " + object_name(previous_object()));
}

/*
	A connection has just been opened for this object.
	For a binary connection, if the return value of this function is
	non-zero, a UDP channel will be opened for the same object.  UDP
	datagrams can be received from the same host and port as the TCP
	connection is on, and after at least one datagram has been
	received, send_datagram() can be used to send datagrams to that
	destination.
	For a telnet connection, the return value will be ignored.
*/
static nomask int open()
{
	Driver->msgs(">> CONNECT",query_ip_name(this_object()),query_ip_number(this_object()));
	if (binary) { destruct(); return 0; }
	prompt = "Who are you? ";
	write( "\n  ~~~ Welcome ~~~\n");
	write(prompt,1);
}

void save()
{
	if (!savable_user) return;
	last_ip = query_ip_number(this_object());
	last_on = time();
	save_object(UserDataDir + user_name + ".sav");
	if (player) player->save();
}

void quit(varargs int reconnect)
{
	quitting = 1;
	save();
	/* delete player stuff here */
	if (player && !reconnect) player->quit();
	destruct();
}

static nomask int handle_user_command(string str)
{
	string cmd, args;
	if (str == "quit") {
		quit();
		return 1;
	}
	if (str == "who") {
		int i;
		object *us, u;
		string s, ret;

		us = users();
		ret = "";
		for (i=sizeof(us); i--; ) {
			u = us[i];
			s = u->query_user_name();
			if (s == "") continue;
			if (ret != "") ret += ", ";
			ret += s;
		}
		if (ret == "") ret = "Looks like it's only you.";
		else {
			ret = "Other users: " + ret;
		}
		write(ret);
		return 1;
	}
	if (str == "cls") {
		write(AnsiCls,1);
		return 1;
	}
	if (str == "colors") {
		int i;
		for (i=0; i<256; i++) {
			if (i==16) write(" ");
			else if (i>16 && !((i-16)%36)) write(" ");
			write(Ansi("48;5;"+i) + " " + AnsiReset,1);
		}
		write(AnsiReset + "\n");
		write("\033Z");
		return 1;
	}
	if (str == "resize") {
		string s;
	/*	s = Esc+"7"+Esc+"[999;999f"+Esc+"[6n"+Esc+"8"; */
	/*	s = "\0337\033[999;999f\033[6n\0338"; */
		s = "\0337\033[999;999f\033[6n\0338";
		write(s,1);
		write("[Press Enter to Continue] ",1);
		input_to("resize2",1);
		return 1;
	}

	return 0;
}

static object input_to_obj;
static string input_to_func;

void set_input_to(object o, string f)
{
	if(o && f && function_object(f, o)) {
		input_to_obj = o;
		input_to_func = f;
	} else {
		error("input_to target does not exist.");
	}
}
int in_input_to() { return !!input_to_obj; }

static int screen_rows, screen_columns;

int query_rows() { return screen_rows ? screen_rows : 22; }
int query_columns() { return screen_columns ? screen_columns : 80; }

void resize2(string response) {
	int row,col;
	write (" response has " + strlen(response) + " charaters");
	if (sscanf(response,"%*s[%d;%dR",row,col) == 3)
	write("\nDetected " + col + "x" + row  + " display.");
	screen_rows = row; screen_columns = col;
}


/*
	This function is called with the text the user typed.  For telnet
	connections, this is always an entire line, with the newline at
	the end filtered out.  For binary connections, this is whatever
	the other side of the connection saw fit to send, without any
	filtering or processing.
*/
static nomask void receive_message(string str)
{
	string *cmd, s;
	int c, did_cmd;

	last_input = time();

	if (no_echo) echo(1); /* echo(0) only lasts one input */

	if (login_state != LoginDone)
	{
		handle_login(str);
		if (login_state != LoginDone) return;

		if (!player) {
			string s;
			s = "/P/" + user_name;
			if (file_size(s + ".sav") == -1) {
				write_file(s + ".sav","");
			}
			player = Driver->find_player(user_name);
			if (!player) {

			}
			player->set_user(this_object());
		}
		savable_user = 1;
		save();
		if (!wizsoul) {
			wizsoul = clone_object(find_object(WizSoul));
			wizsoul->set_owner(WizHome(user_name));
			wizsoul->set_wizname(user_name);
		}
		if (!player->environment()) {
			player->__move(find_object(Nowhere));
		}
		str = "resize";
		/* fall through */
	}

	if (input_to_obj) {
		object o; string s;
		o = input_to_obj; s = input_to_func;
		input_to_obj = nil;  input_to_func = nil;
		call_other(o, s, str);
	} else {
		did_cmd = handle_user_command(str);
		if (!did_cmd && wizsoul) {
			did_cmd = wizsoul->handle_wizard_command(str);
		}
		if (!did_cmd) {
			player->do_command(str);
		}
	}

	if (quitting) return;

	write(AnsiReset,1);
	if (input_to_obj) {
		/* no prompt */
	} else if (wizsoul && (s = query_editor(wizsoul))) {
		if (s == "command")
			write(":",1);
		else
			write("*\b",1);
	} else {
		write(player->query_prompt(),1);
	}

}

/*
	The connection for this object has just been closed.  close() is
	called when the user goes linkdead, or when the user object is
	destructed.  The flag argument is 1 if the object is destructed,
	0 otherwise.
*/
void close(int flag)
{
	if (wizsoul) wizsoul->destruct();
	if (user_name) write("\nGoodbye, " + user_name + "!\n");
}


/*
	This function is called when a string sent by send_message was
	fully transmitted.
*/
void message_done()
{
}
