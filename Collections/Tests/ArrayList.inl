
TEST_CASE("ArrayList construction", "[array-list]")
{
    using namespace NamespaceName__;

    SECTION("List must be initially empty and unallocated")
    {
        ArrayList<int> ints;

        REQUIRE(0 == ints.Count());
        REQUIRE(0 == ints.Capacity());
        REQUIRE(nullptr == ints.Data());
        REQUIRE(0 == ints.DataSize());
        REQUIRE(0 == ints.AllocatedSize());
    }

    SECTION("List init with capacity should adequately allocate")
    {
        const auto n = 200;
        ArrayList<int> ints(n);

        REQUIRE(0 == ints.Count());
        REQUIRE(n == ints.Capacity());
        REQUIRE(nullptr != ints.Data());
        REQUIRE(0 == ints.DataSize());
        REQUIRE(n * sizeof(int) == ints.AllocatedSize());
    }

    SECTION("List construction from init list")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };

        for (auto i = 0U; i < a.Count(); i++)
            REQUIRE(a[i] == i);
    }

    SECTION("List assign from init list")
    {
        ArrayList<int> a;

        a = { 0, 1, 2, 3, 4, 5, 6 };

        for (auto i = 0U; i < a.Count(); i++)
            REQUIRE(a[i] == i);
    }
}

TEST_CASE("ArrayList set capacity", "[array-list]")
{
    using namespace NamespaceName__;

    SECTION("Set capacity larger than 0")
    {
        ArrayList<int> ints;

        ints.SetCapacity(10);

        REQUIRE(ints.Count() == 0);
        REQUIRE(ints.Capacity() == 10);
    }

    SECTION("Set capacity to smaller")
    {
        ArrayList<int> ints(20);

        ints.SetCapacity(10);

        REQUIRE(ints.Count() == 0);
        REQUIRE(ints.Capacity() == 10);
    }

    SECTION("Set capacity to larger with count > 0")
    {
        ArrayList<int> a = { 1, 2, 3, 4, 5 };
        ArrayList<int> b = { 1, 2, 3, 4, 5 };

        a.SetCapacity(20);

        REQUIRE(a.Capacity() == 20);
        REQUIRE(a.CompareTo(b));
    }

    SECTION("Set capacity to smaller with count > 0")
    {
        ArrayList<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        ArrayList<int> b = { 1, 2, 3 };

        a.SetCapacity(b.Count());

        REQUIRE(a.Capacity() == b.Count());
        REQUIRE(a.CompareTo(b));
    }
}

TEST_CASE("ArrayList reserve", "[array-list]")
{
    using namespace NamespaceName__;

    SECTION("Reserve after default construct")
    {
        ArrayList<int> ints;

        ints.Reserve(10);

        REQUIRE(ints.Data() != nullptr);
        REQUIRE(ints.Count() == 0);
        REQUIRE(ints.Capacity() >= 10);
    }

    SECTION("Reserve smaller")
    {
        ArrayList<int> ints(10);

        ints.Reserve(0);

        REQUIRE(ints.Data() != nullptr);
        REQUIRE(ints.Count() == 0);
        REQUIRE(ints.Capacity() >= 0);
    }

    SECTION("Reserve larger")
    {
        ArrayList<int> ints(10);

        ints.Reserve(20);

        REQUIRE(ints.Data() != nullptr);
        REQUIRE(ints.Count() == 0);
        REQUIRE(ints.Capacity() >= 20);
    }
}

TEST_CASE("ArrayList add", "[array-list]")
{
    using namespace NamespaceName__;

    SECTION("Add, no growth")
    {
        ArrayList<int> ints(100);

        for (auto i = 0; i < 50; i++)
            ints.Add(0);

        REQUIRE(ints.Count() == 50);
        REQUIRE(ints.Capacity() >= 100);
    }

    SECTION("Add with growth")
    {
        ArrayList<int> ints(10);

        for (auto i = 0; i < 20; i++)
            ints.Add(0);

        REQUIRE(ints.Count() == 20);
        REQUIRE(ints.Capacity() >= 20);
    }

    SECTION("Add, no init capacity")
    {
        ArrayList<int> ints;

        for (auto i = 0; i < 10; i++)
            ints.Add(0);

        REQUIRE(ints.Count() == 10);
        REQUIRE(ints.Capacity() >= 10);
    }

    SECTION("Add values")
    {
        ArrayList<int> ints(10);

        for (auto i = 0; i < ints.Count(); i++)
            ints.Add(i);

        for (auto i = 0; i < ints.Count(); i++)
            REQUIRE(ints[i] == i);
    }
}

TEST_CASE("ArrayList insert", "[array-list]")
{
    using namespace NamespaceName__;

    SECTION("Insert in the middle")
    {
        ArrayList<int> a = { 0, 1,     2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, -2, 2, 3, 4, 5, 6 };

        a.Insert(2, -2);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Insert at start")
    {
        ArrayList<int> a = {     0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { -1, 0, 1, 2, 3, 4, 5, 6 };

        a.Insert(0, -1);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Insert at end")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 2, 3, 4, 5, 6, -1 };

        a.Insert(a.Count(), -1);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Insert into empty")
    {
        ArrayList<int> a;

        a.Insert(0, 13);

        REQUIRE(a[0] == 13);
        REQUIRE(a.Count() == 1);
        REQUIRE(a.Capacity() >= 1);
    }
}

TEST_CASE("ArrayList removal", "[array-list]")
{
    using namespace NamespaceName__;

    SECTION("Removal from the middle")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1,    3, 4, 5, 6 };

        a.RemoveAt(2);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Removal from the start")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = {    1, 2, 3, 4, 5, 6 };

        a.RemoveAt(0);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Removal from the end")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 2, 3, 4, 5 };

        a.RemoveAt(a.Count() - 1);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Unsorted removal from the middle")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 6, 3, 4, 5 };

        a.RemoveAtSwapBack(2);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Unsorted removal from the start")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 6, 1, 2, 3, 4, 5 };

        a.RemoveAtSwapBack(0);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Unsorted removal from the end")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 2, 3, 4, 5 };

        a.RemoveAtSwapBack(a.Count() - 1);

        REQUIRE(b.CompareTo(a));
    }
}

TEST_CASE("ArrayList resize", "[array-list]")
{
    using namespace NamespaceName__;

    ArrayList<int> ints;

    ints.Resize(20);

    REQUIRE(ints.Count() == 20);
    REQUIRE(ints.Capacity() >= 20);

    ints.Resize(10);

    REQUIRE(ints.Count() == 10);
    REQUIRE(ints.Capacity() >= 10);

    ints.Resize(30);

    REQUIRE(ints.Count() == 30);
    REQUIRE(ints.Capacity() >= 30);
}