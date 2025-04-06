#ifdef ENABLE_HACK
#ifndef _H_UGLY_STACK
#define _H_UGLY_STACK
#define P_0()
#define P_1(x)                PUSH(x);
#define P_2(x, y)             PUSH(x); PUSH(y); 
#define P_3(x, y, z)          PUSH(x); PUSH(y); PUSH(z);
#define P_4(x, y, z, w)       PUSH(x); PUSH(y); PUSH(z); PUSH(w);
#define P_6(x, y, z, w, r, t) PUSH(x); PUSH(y); PUSH(z); PUSH(w); PUSH(r); PUSH(t); 
#define EFF_1(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w1); __VA_ARGS__; end; }
#define EFF_2(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w2); INIT(w1); __VA_ARGS__; end; }
#define EFF_3(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w3); INIT(w2); INIT(w1); __VA_ARGS__; end; }
#define EFF_4(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w4); INIT(w3); INIT(w2); INIT(w1); __VA_ARGS__; end; }
#define EFF_6(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w6); INIT(w5); INIT(w4); INIT(w3); INIT(w2); INIT(w1); __VA_ARGS__; end; }

#define RP_0()
#define RP_1(x)                RPUSH(x);
#define RP_2(x, y)             RPUSH(x); RPUSH(y); 
#define RP_3(x, y, z)          RPUSH(x); RPUSH(y); RPUSH(z);
#define RP_4(x, y, z, w)       RPUSH(x); RPUSH(y); RPUSH(z); RPUSH(w);
#define RP_6(x, y, z, w, r, t) RPUSH(x); RPUSH(y); RPUSH(z); RPUSH(w); RPUSH(r); RPUSH(t); 
#define REFF_1(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w1); __VA_ARGS__; end; }
#define REFF_2(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w2); INIT(w1); __VA_ARGS__; end; }
#define REFF_3(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w3); INIT(w2); INIT(w1); __VA_ARGS__; end; }
#define REFF_4(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w4); INIT(w3); INIT(w2); INIT(w1); __VA_ARGS__; end; }
#define REFF_6(_name, start, end, ...) MAKEPRIM(_name){ start; INIT(w6); INIT(w5); INIT(w4); INIT(w3); INIT(w2); INIT(w1); __VA_ARGS__; end; }

#endif
#endif
