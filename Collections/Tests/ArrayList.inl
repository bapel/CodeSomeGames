
TEST_CASE("List construction", "[list]")
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

TEST_CASE("List add", "[list-construct]")
{
    using namespace NamespaceName__;

    SECTION("List Add with no growth")
    {
        const auto n = 100;
        ArrayList<int> ints(n);

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i);

        REQUIRE(n == ints.Count());
        REQUIRE(n == ints.Capacity());
        REQUIRE(n * sizeof(int) == ints.DataSize());
        REQUIRE(n * sizeof(int) == ints.AllocatedSize());

        for (auto i = 0; i < n; i++)
            REQUIRE(ints[i] == (2 * i));
    }

    SECTION("List Add with growth")
    {
        const auto n0 = 10;
        const auto n = 200;
        ArrayList<int> ints(10);

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i);

        REQUIRE(n == ints.Count());
        REQUIRE(n <= ints.Capacity());
        REQUIRE(n * sizeof(int) == ints.DataSize());
        REQUIRE(n * sizeof(int) <= ints.AllocatedSize());

        for (auto i = 0; i < n; i++)
            REQUIRE(ints[i] == (2 * i));
    }

    SECTION("List Add, no init capacity")
    {
        const auto n = 100;
        ArrayList<int> ints;

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i);

        REQUIRE(n == ints.Count());
        REQUIRE(n <= ints.Capacity());
        REQUIRE(n * sizeof(int) == ints.DataSize());
        REQUIRE(n * sizeof(int) <= ints.AllocatedSize());

        for (auto i = 0; i < n; i++)
            REQUIRE(ints[i] == (2 * i));
    }
}

TEST_CASE("List insert and retrieval", "[list-insert]")
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

        a.Insert(0, 0);

        REQUIRE(0 == a[0]);
        REQUIRE(1 == a.Count());
        REQUIRE(1 <= a.Capacity());
        REQUIRE(sizeof(int) == a.DataSize());
        REQUIRE(sizeof(int) <= a.AllocatedSize());
    }
}

TEST_CASE("List removal", "[list-remove]")
{
    using namespace NamespaceName__;

    SECTION("Removal from the middle")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1,    3, 4, 5, 6 };

        a.Remove(2);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Removal from the start")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = {    1, 2, 3, 4, 5, 6 };

        a.Remove(0);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Removal from the end")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 2, 3, 4, 5 };

        a.Remove(a.Count() - 1);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Unsorted removal from the middle")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 6, 3, 4, 5 };

        a.RemoveUnsorted(2);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Unsorted removal from the start")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 6, 1, 2, 3, 4, 5 };

        a.RemoveUnsorted(0);

        REQUIRE(b.CompareTo(a));
    }

    SECTION("Unsorted removal from the end")
    {
        ArrayList<int> a = { 0, 1, 2, 3, 4, 5, 6 };
        ArrayList<int> b = { 0, 1, 2, 3, 4, 5 };

        a.RemoveUnsorted(a.Count() - 1);

        REQUIRE(b.CompareTo(a));
    }
}