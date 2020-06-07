#define BT__(N__) NamespaceName__::BitSet<N__>
#define BitSetTypes__ BT__(8), BT__(16), BT__(24), BT__(32), BT__(64), BT__(128), BT__(256)

TEMPLATE_TEST_CASE("Bitset init", "[bitset]", BitSetTypes__)
{
    using namespace NamespaceName__;

    SECTION("Default construct")
    {
        TestType a;

        REQUIRE(a.Any() == false);
        REQUIRE(a.All() == false);
        REQUIRE(a.None() == true);
    }

    SECTION("Init from ints. All ones")
    {
        TestType a = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };

        REQUIRE(a.Any() == true);
        REQUIRE(a.All() == true);
        REQUIRE(a.None() == false);
    }

    SECTION("Init from int array. All ones")
    {
        uint64_t ints[] = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
        TestType a(ints, 4);

        REQUIRE(a.Any() == true);
        REQUIRE(a.All() == true);
        REQUIRE(a.None() == false);
    }
}

TEMPLATE_TEST_CASE("Get/Set bit", "[bitset]", BitSetTypes__)
{
    using namespace NamespaceName__;

    TestType a;

    REQUIRE(a.Any() == false);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == true);

    a.Set(3);
    REQUIRE(true == a[3]);

    a.Set(a.Count() / 3);
    REQUIRE(true == a[a.Count() / 3]);

    a.Unset(3);
    REQUIRE(false == a[3]);

    a.Unset(a.Count() / 3);
    REQUIRE(false == a[a.Count() / 3]);

    REQUIRE(a.Any() == false);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == true);
}

TEMPLATE_TEST_CASE("Clear bitset", "[bitset]", BitSetTypes__)
{
    using namespace NamespaceName__;

    TestType a;

    REQUIRE(false == a[3]);
    a.Set(3);
    REQUIRE(true == a[3]);

    REQUIRE(false == a[a.Count() / 3]);
    a.Set(a.Count() / 3);
    REQUIRE(true == a[a.Count() / 3]);

    REQUIRE(a.Any() == true);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == false);

    a.Clear();

    REQUIRE(a.Any() == false);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == true);
}

TEST_CASE("Bitset operators", "[bitset]")
{
    using namespace NamespaceName__;

    SECTION("& operator")
    {
        BitSet<100> a = { 858302ull, 78267498ull };
        BitSet<100> b = { 927495ull, 95475902ull };
        BitSet<100> c = 
        {
              858302ull & 927495ull,
            78267498ull & 95475902ull
        };

        REQUIRE((a & b) == c);
    }

    SECTION("| operator")
    {
        BitSet<100> a = { 858302ull, 78267498ull };
        BitSet<100> b = { 927495ull, 95475902ull };
        BitSet<100> c = 
        {
              858302ull | 927495ull,
            78267498ull | 95475902ull
        };

        REQUIRE((a | b) == c);
    }

    SECTION("^ operator")
    {
        BitSet<100> a = { 858302ull, 78267498ull };
        BitSet<100> b = { 927495ull, 95475902ull };
        BitSet<100> c = 
        {
              858302ull ^ 927495ull,
            78267498ull ^ 95475902ull
        };

        REQUIRE((a ^ b) == c);
    }

    SECTION("~ operator")
    {
        BitSet<100> a = {  858302ull,  78267498ull };
        BitSet<100> b = { ~858302ull, ~78267498ull };

        REQUIRE((~a) == b);
    }

    SECTION("<< operator")
    {
        BitSet<100> a = { 0xFFFF'FFFF'FFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF };
        BitSet<100> b = { 0xFFFF'FFFF'FFFF'0000, 0xFFFF'FFFF'FFFF'FFFF }; // a << 16
        BitSet<100> c = { 0xFFFF'FFFF'FF00'0000, 0xFFFF'FFFF'FFFF'FFFF }; // a << 24
        BitSet<100> d = {                     0, 0xFFFF'FFFF'FFFF'FF00 }; // a << (18 * 4 = 72)

        REQUIRE((a <<  16) == b);
        REQUIRE((a <<  24) == c);
        REQUIRE((a <<  72) == d);
        REQUIRE((a << 150) == 0);
    }

    SECTION(">> operator")
    {
        BitSet<100> a = { 0xFFFF'FFFF'FFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF };
        BitSet<100> b = { 0xFFFF'FFFF'FFFF'FFFF, 0x0000'FFFF'FFFF'FFFF }; // a >> 16
        BitSet<100> c = { 0xFFFF'FFFF'FFFF'FFFF, 0x0000'00FF'FFFF'FFFF }; // a >> 24
        BitSet<100> d = { 0x00FF'FFFF'FFFF'FFFF, 0                     }; // a >> (18 * 4 = 72)

        REQUIRE((a >>  16) == b);
        REQUIRE((a >>  24) == c);
        REQUIRE((a >>  72) == d);
        REQUIRE((a >> 150) == 0);
    }
}

#undef BT__
#undef BitSetTypes__