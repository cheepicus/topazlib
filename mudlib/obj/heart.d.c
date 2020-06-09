
object **timer_s;
int tail_s;

int request_heartbeat(int seconds)
{
	object o;

	if (seconds < 1 || seconds > 59) error("seconds out of range 1..59")
	o = previous_object();
	if (!timer_s) timer_s = allocate(60);
	if (!timer_s[seconds]) timer_s[seconds] = ({ });
	timer_s[seconds] += ({ o });
}

void do_seconds()
{
	int i,j, ret;
	object *objs,o;
	for (i=1; i<tail_s; i++) {
		int sz;
		objs = timer_s[i];
		if (!objs || !(sz=sizeof(objs))) continue;
		for (j=0; j<sz; j++) {
			o = objs[j];
			if (!o) continue;
			ret = 0;
			catch { ret = o->timer(); }
			if (!ret) objs[j] = nil;
		}
	}
}
