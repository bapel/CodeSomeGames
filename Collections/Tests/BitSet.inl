
TEST_CASE("Bitset init", "[bitset]")
{
    using namespace NamespaceName__;

    SECTION("Default construct")
    {
        BitSet<100> a;
        for (auto i = 0U; i < a.Count(); i++)
            REQUIRE(a[i] == false);

        REQUIRE(a.Any() == false);
        REQUIRE(a.All() == false);
        REQUIRE(a.None() == true);
    }

    SECTION("Init from ints. All ones")
    {
        BitSet<100> a({ 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF });

        REQUIRE(a.Any() == true);
        REQUIRE(a.All() == true);
        REQUIRE(a.None() == false);
    }

    SECTION("Init from int array. All ones")
    {
        uint64_t ints[] = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
        BitSet<100> a(ints, sizeof(ints) / sizeof(uint64_t));

        REQUIRE(a.Any() == true);
        REQUIRE(a.All() == true);
        REQUIRE(a.None() == false);
    }
}

TEST_CASE("Get/Set bit", "[bitset]")
{
    using namespace NamespaceName__;

    BitSet<100> a;

    REQUIRE(a.Count() >= 100);
    REQUIRE(a.Count() <= 128);

    REQUIRE(false == a[3]);
    a.Set(3, true);
    REQUIRE(true == a[3]);

    REQUIRE(false == a[70]);
    a.Set(70, true);
    REQUIRE(true == a[70]);

    REQUIRE(a.Any() == true);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == false);
}

TEST_CASE("Clear bitset", "[bitset]")
{
    using namespace NamespaceName__;

    BitSet<100> a;

    REQUIRE(a.Count() >= 70);
    REQUIRE(a.Count() <= 128);

    REQUIRE(false == a[3]);
    a.Set(3, true);
    REQUIRE(true == a[3]);

    REQUIRE(false == a[65]);
    a.Set(65, true);
    REQUIRE(true == a[65]);

    REQUIRE(a.Any() == true);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == false);

    a.Clear();

    REQUIRE(a.Any() == false);
    REQUIRE(a.All() == false);
    REQUIRE(a.None() == true);
}