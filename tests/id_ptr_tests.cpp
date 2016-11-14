#include <gtest/gtest.h>

#include "core/Idptr.h"

#include "core/objects/MainObject.h"
#include "core/objects/test/UnsyncGenerator.h"

class TempTable
{
public:
    TempTable()
    {
        old_table_ = id_ptr_id_table;
        id_ptr_id_table = &table_;
        table_.resize(100);
    }
    ~TempTable()
    {
        id_ptr_id_table = old_table_;
    }

private:
    std::vector<ObjectInfo> table_;
    std::vector<ObjectInfo>* old_table_;
};

TEST(IdPtrDeathTest, Death)
{
    TempTable table;

    ASSERT_DEATH(
    {
        IdPtr<IMainObject> object(101);
        object.IsValid();
    }, "Id table lookup fail");
}

class IdPtrTest : public ::testing::Test
{
private:
    TempTable table_;
};

TEST_F(IdPtrTest, Constructors)
{
    IdPtr<IMainObject> object(40);
    ASSERT_EQ(object.Id(), 40);

    object = 10;
    ASSERT_EQ(object.Id(), 10);

    IdPtr<IMainObject> empty;
    ASSERT_EQ(empty.Id(), 0);

    empty = object;
    ASSERT_EQ(empty.Id(), 10);

    IdPtr<IMainObject> from_other(empty);
    ASSERT_EQ(from_other.Id(), 10);

    IdPtr<UnsyncGenerator> gen_from_other(from_other);
    ASSERT_EQ(from_other.Id(), 10);

    from_other = 33;
    gen_from_other = from_other;
    ASSERT_EQ(from_other.Id(), 33);
}

TEST_F(IdPtrTest, EqualOperator)
{
    IdPtr<IMainObject> object(23);
    ASSERT_TRUE(object == object);
    ASSERT_FALSE(object != object);

    IdPtr<IMainObject> other(22);
    ASSERT_FALSE(object == other);
    ASSERT_FALSE(other == object);
    ASSERT_TRUE(object != other);
    ASSERT_TRUE(other != object);

    IdPtr<IMainObject> object2(23);
    ASSERT_TRUE(object == object2);
    ASSERT_TRUE(object2 == object);
    ASSERT_FALSE(object != object2);
    ASSERT_FALSE(object2 != object);
}

TEST_F(IdPtrTest, Dereference)
{
    IMainObject object(42);
    (*id_ptr_id_table)[42].object = &object;

    IdPtr<IMainObject> ptr;
    ASSERT_EQ(ptr.operator*(), nullptr);

    ptr = 10;
    ASSERT_EQ(ptr.operator*(), nullptr);

    ptr = 42;
    ASSERT_EQ(ptr.operator*(), &object);
    // Cache
    ASSERT_EQ(ptr.operator*(), &object);

    ASSERT_EQ(ptr.operator*(), ptr.operator->());
}

TEST_F(IdPtrTest, Validating)
{
    IMainObject object(42);
    (*id_ptr_id_table)[42].object = &object;

    IdPtr<IMainObject> ptr;
    ASSERT_FALSE(ptr.IsValid());
    ASSERT_FALSE(ptr);

    ptr = 10;
    ASSERT_FALSE(ptr.IsValid());

    ptr = 42;
    ASSERT_TRUE(ptr.IsValid());
    ASSERT_TRUE(ptr);

    (*id_ptr_id_table)[42].object = nullptr;
    ASSERT_FALSE(ptr.IsValid());
}

TEST_F(IdPtrTest, SaveAndLoad)
{
    IdPtr<IMainObject> ptr(93);
    std::stringstream str;
    str << ptr;
    ASSERT_EQ(str.str(), "93 ");

    IdPtr<IMainObject> ptr2;
    ASSERT_FALSE(ptr2 == ptr);

    str >> ptr2;
    ASSERT_TRUE(ptr2 == ptr);
}







