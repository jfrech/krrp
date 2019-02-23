#include "atom.h"
#include "util.h"
#include "parse.h"
#include "interpret.h"
#include "memorymanagement.h"

#include <stdlib.h>
#include <stdio.h>

void test(const char *source, Atom *expected) {
    char *esource = stresc(source);

    AtomList *parsed = parse(source);
    Atom *scope = main_scope();
    Atom *computed = interpret(0, parsed, scope, true);

    if (!atom_equal(computed, expected))
        printf("[FAIL] \"%s\"\n   :: '%s' differs from expected '%s'.\n", esource, atom_repr(computed), atom_repr(expected));
    else {
        /*printf("[pass] \"%s\"\n", esource);
        printf(".> %s\n", atom_repr(computed));
        // */
    }

    atomlist_free(parsed);
    free(esource);
}

void test_all() {
    Atom *T = atom_integer_new(1);

    test(" ~hello\n!f$123.![name!]8![??]^abc:++abc.[??]912", atom_integer_new(12));
    //test("!f^ab:+ab.!b8", atom_null_new());
    //parse_failure("^~hello\nab$c:.");
    //parse_failure("!!!!f^ab[ab]:+!+[ab]ab.!z!z");
    test("1", atom_integer_new(1));
    test("+56", atom_integer_new(11));
    test("<*+123$10.", T);
    test(",;,;;,,;+13", atom_integer_new(4));
    test("!a8!b9!c+ba*cc", atom_integer_new(289));
    test(",^n:,^n:+n1.n.1", atom_integer_new(2));
    //test(",,^n:!k+1n^:+k1..1", atom_integer_new(3));
    //test("!k4+,,^n:!k+1n^:+k1..1k", atom_integer_new(7));
    test("!f^n:+n1. f8", atom_integer_new(9));
    //interpret_failure("?")
    test("!f^g:gg123. f,;,;;+", atom_integer_new(6));
    test("!f^n:?n*n@-n11. f9f8", atom_integer_new(362880));
    test(",^n:?n*n@-n11.*55", atom_integer_new(7034535277573963776));
    test("![fib]^n:?<n21+@-n1@-n2. [fib]$20.", atom_integer_new(10946));
    test("$1.", atom_integer_new(1));
    //parse_failure("^:.")
    test("!f^_:f_. ?0?0f989", atom_integer_new(9));
    test("!f^_:f_. ?1?0f548", atom_integer_new(4));
    test("!P^n:!p^nd:?>d1?%ndpn-d101.?>n1pn-n10. P$101.", atom_integer_new(1));
    test("!\377^[\n]:^[\t]:+[\t][\n].. !\"9!\276\3778 \276\"", atom_integer_new(17));
    test("!f^n:?n*n@-n11. f0f1f2f3f4f5f6f7f8f9f+55f+65f+66f+76f+77f+87f+88", atom_integer_new(1));
    // OLD "!C#fr.!E#. C0E"
    test("?19?0?0?0000?123", atom_integer_new(9)); // TODO :: check
    // (design todo): infinite recursion exit -- impossible
    //"!f^n:fn. ?1f01"
    test("!f^n:?n*nf-n11. f4", atom_integer_new(24));
    test("!f^n:?>n1+f-n1f-n21. f0f1f2f3f4f5f6f7f8f9f+55", atom_integer_new(1));
    // error "!C#fr.C"
    test(",^n:?n*n@-n11.5", atom_integer_new(120));
    // idea "![struct_caller]#[struct_type][arg0][...][argn]."
    // "#?[struct_type][obj]"
    // "#![obj][arg]"
    test("!Y#E. !Z#Lfr. !QZ1Z2Z3Y #!fQ #!f#!rQ #!f#!r#!rQ #?E#!r#!r#!rQ", atom_integer_new(1));




    test("!E#E. !L#Lfr. !M^gl:?#?El E Lg#!flM;g#!rl.        !sL2L3L5E !tE  = t M^n:*2n.t", T);
    test("!E#E. !L#Lfr. !M^gl:? #?El E L g#!fl M;g#!rl.       !sL2L3L5E !tE ~s\n !G^n:*3n. = M;Gs L6L9L$15.E", T);
    // odd "!E#E. !L#Lfr. !M^gl:.        !sL2L3L5E !tE  t M^n:*2n.t"
    test("!F^g:g8. !h^n:*2n. F^n:+n1.", atom_integer_new(9));
    test("!f^n:?n*n@-n11. f8", atom_integer_new(40320));
    test("!f^n:?<n2n+@-n1@-n2. f0f1f2f3f4f5f6f7f8", atom_integer_new(0));
    test("!E#E. !L#Lfr. !M^gl:?#?ElELg#!flM;g#!rl. = M^n:*nn.L1L2L3L4E L1L4L9L$16.E", T);
    test("!L#Lfr. !E#E. !tL1L5E !M^gl:?#?ElELg#!flM;g#!rl. !sL2L6E =M^n:+n1.t s", T);
    test("!E#E. !L#Lfr. !tL1E =tL1E", T);
    test("!E#E.!L#Lfr. ~range\n!R^nm:?=nmELnR+n1m. ~map\n!M^gl:?#?ElELg#!fl@;g#!rl. ~filter\n!F^pl:?#?ElE?p#!flL#!fl@;p#!rl@;p#!rl. ~zip\n!Z^lr:?#?ElE?#?ErELL#!flL#!frE@#!rl#!rr. ![res] Z R04 F^n:%n2.M^n:*nn.R38 ~\n=[res]LL0L9ELL1L$25.ELL2L$49.EE", T);
    test("!f^n:*n3. !g^n:*n3. f3g3g7f7g8f8", atom_integer_new(9));
    // untestable ";-^n:?<n2n@-n1-n2."
    test("!T#Tab. -01 T12 T21 T12", atom_integer_new(-1));
    test("!F^a:^b:*+bab.. !_F8 ,F31 ~=;F^:.", atom_integer_new(4));
    test("!L#Lfr. !E#E. !tL1E =,^s:L2s.t L2L1E", T);
    test("!E#E. !L#Lfr. !f^n:?<n2n+@-n1@-n2. !M^gl:?#?ElELg#!flM;g#!rl. !R^ab:?=abELaR+a1b. !rM;fR09 !eL0L1L1L2L3L5L8L$13.L$21.E   ~re\n =re", T);
    test("!E#E.!L#Lfr. !R^ab:?=abELaR+a1b. !p^n: !f^nd:?=d11?%ndfn-d10. ?<n20fn-n1. !F^pl:?#?ElE?p#!flL#!flF;p#!rlF;p#!rl. = F;pR0$99. L2L3L5L7L$11.L$13.L$17.L$19.L$23.L$29.L$31.L$37.L$41.L$43.L$47.L$53.L$59.L$61.L$67.L$71.L$73.L$79.L$83.L$89.L$97.E", T);
    test("!_#_.!'#''_. !R^ab:?=ab_'aR+a1b. !M^gl:?#?_l_'g#!'lM;g#!_l. = M^n:*2n.R-09+91 '$-18.'$-16.'$-14.'$-12.'$-10.'$-8.'$-6.'$-4.'$-2.'0'2'4'6'8'$10.'$12.'$14.'$16.'$18._", T);

    test("!Z#Z.!S#Sp. ![->]^n:?nS@-n1Z. = [->]4 SSSSZ", T);
    test("!Z#Z.!S#Sp. ![<-]^n:?#?Zn0+1[<-]#!pn. = [<-]SSSSSSZ 6", T);
    test("![and]^pq:?p?q100. !f^c:^tf:?ctf.. ![decision0]f0 ![decision1]f1   [and] =[decision0]488 =[decision1]484", T);
    test("~An implementation of Peano naturals.\n !Z#Z.!S#Sp. ![->]^n:?nS@-n1Z. ![<-]^n:?#?Zn0+1@#!pn. ~TEST OVERLOADING HERE\n ![p+]^nm:?#?ZnmS@#!pnm. =[<-][p+][->]3[->]8$11.", T);
    test("![factorial]^n:?n*n@-n11. ![choose]^nk:!f;[factorial]/fn*fkf-nk. [choose]83", atom_integer_new(56));

    //printf("\n");

    // other char *str = stresc(source);printf("Source code \"%s\"\n", str);free(str);printf(".> %s\n", str=stresc("Hello\\worl\077d.\x37Ho\xafw?\n\thj\xfekl"));free(str);
}
