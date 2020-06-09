
inherit "/obj/room.inh";

void make(string ofile, string id, string short)
{
	object o;
	if(!find_object(ofile)) call_other(ofile,"???");
	o = clone_object(find_object(ofile));
	o->add_ids(short, id);
	o->__move(this_object());
}

void create() {
	object o;

	::create();
	make("/obj/item", "key", "blue key");
	make("/obj/item", "key", "red key");
	make("/obj/item", "key", "yellow key");
	make("/obj/item", "stick", "stick");
	make("/obj/item", "apple", "apple");
	make("/obj/item", "graymist", "Graymist");
	make("/obj/item", "key", "black key");
	make("/obj/item", "key", "white key");

	set_light(1);
}

string short() { return "Nowhere"; }

string long() { return "A place between all the other places."; }
