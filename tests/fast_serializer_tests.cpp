#include <gtest/gtest.h>

#include "FastSerializer.h"

#include <QFile>

TEST(FastSerializer, Constructor)
{
    FastSerializer serializer;
    ASSERT_EQ(serializer.GetIndex(), 0);
    ASSERT_TRUE(serializer.GetData());
}

TEST(FastSerializer, WriteBool)
{
    FastSerializer serializer;

    serializer.Write(true);
    ASSERT_EQ(serializer.GetIndex(), 1);
    EXPECT_TRUE(serializer.GetData()[0]);

    serializer.Write(false);
    ASSERT_EQ(serializer.GetIndex(), 2);
    EXPECT_FALSE(serializer.GetData()[1]);
}

TEST(FastSerializer, WriteInt32)
{
    FastSerializer serializer;

    {
        qint32 value = 0;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 4);
        for (int i = 0; i < 4; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\x00');
        }
    }

    {
        qint32 value = 1;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 8);
        EXPECT_EQ(serializer.GetData()[4], '\x01');
        for (int i = 5; i < 8; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\x00');
        }
    }

    {
        qint32 value = -1;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 12);
        for (int i = 8; i < 12; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\xFF');
        }
    }

    {
        qint32 value = 2047583047;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 16);
        EXPECT_EQ(serializer.GetData()[12], '\x47');
        EXPECT_EQ(serializer.GetData()[13], '\xA3');
        EXPECT_EQ(serializer.GetData()[14], '\x0B');
        EXPECT_EQ(serializer.GetData()[15], '\x7A');
    }

    {
        qint32 value = -2039583948;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 20);
        EXPECT_EQ(serializer.GetData()[16], '\x34');
        EXPECT_EQ(serializer.GetData()[17], '\x6B');
        EXPECT_EQ(serializer.GetData()[18], '\x6E');
        EXPECT_EQ(serializer.GetData()[19], '\x86');
    }
}

TEST(FastSerializer, WriteUint32)
{
    FastSerializer serializer;

    {
        quint32 value = 0;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 4);
        for (int i = 0; i < 4; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\x00');
        }
    }

    {
        quint32 value = 1;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 8);
        EXPECT_EQ(serializer.GetData()[4], '\x01');
        for (int i = 5; i < 8; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\x00');
        }
    }

    {
        quint32 value = 4294967295;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 12);
        for (int i = 8; i < 12; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\xFF');
        }
    }

    {
        quint32 value = 2047583048;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 16);
        EXPECT_EQ(serializer.GetData()[12], '\x48');
        EXPECT_EQ(serializer.GetData()[13], '\xA3');
        EXPECT_EQ(serializer.GetData()[14], '\x0B');
        EXPECT_EQ(serializer.GetData()[15], '\x7A');
    }
}

TEST(FastSerializer, WriteString)
{
    FastSerializer serializer;

    {
        QString value;
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 4);
        for (int i = 0; i < 4; ++i)
        {
            EXPECT_EQ(serializer.GetData()[i], '\x00');
        }
    }

    {
        QString value("Hello world!");
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 32);
        EXPECT_EQ(serializer.GetData()[4], '\x0C');
        EXPECT_EQ(serializer.GetData()[5], '\x00');
        EXPECT_EQ(serializer.GetData()[6], '\x00');
        EXPECT_EQ(serializer.GetData()[7], '\x00');
        EXPECT_EQ(serializer.GetData()[8], 'H');
        EXPECT_EQ(serializer.GetData()[9], '\x00');
        EXPECT_EQ(serializer.GetData()[10], 'e');
        EXPECT_EQ(serializer.GetData()[11], '\x00');
        EXPECT_EQ(serializer.GetData()[18], ' ');
        EXPECT_EQ(serializer.GetData()[19], '\x00');
        EXPECT_EQ(serializer.GetData()[30], '!');
        EXPECT_EQ(serializer.GetData()[31], '\x00');
    }

    {
        QString value = "Привет, мир!";
        serializer.Write(value);
        ASSERT_EQ(serializer.GetIndex(), 60);
        EXPECT_EQ(serializer.GetData()[32], '\x0C');
        EXPECT_EQ(serializer.GetData()[33], '\x00');
        EXPECT_EQ(serializer.GetData()[34], '\x00');
        EXPECT_EQ(serializer.GetData()[35], '\x00');
        EXPECT_EQ(serializer.GetData()[36], '\x1F');
        EXPECT_EQ(serializer.GetData()[37], '\x04');
        EXPECT_EQ(serializer.GetData()[48], ',');
        EXPECT_EQ(serializer.GetData()[49], '\x00');
        EXPECT_EQ(serializer.GetData()[50], ' ');
        EXPECT_EQ(serializer.GetData()[51], '\x00');
        EXPECT_EQ(serializer.GetData()[52], '\x3C');
        EXPECT_EQ(serializer.GetData()[53], '\x04');
        EXPECT_EQ(serializer.GetData()[58], '!');
        EXPECT_EQ(serializer.GetData()[59], '\x00');
    }
}

TEST(FastSerializer, ResetIndex)
{
    FastSerializer serializer;

    ASSERT_EQ(serializer.GetIndex(), 0);
    serializer.Write(42);
    ASSERT_EQ(serializer.GetIndex(), 4);
    serializer.ResetIndex();
    ASSERT_EQ(serializer.GetIndex(), 0);
}

TEST(FastSerializer, BigData)
{
    FastSerializer serializer;

    ASSERT_EQ(serializer.GetIndex(), 0);

    const int BIG_DATA_SIZE = FastSerializer::DEFAULT_SIZE * 2;
    for (int i = 0; i < BIG_DATA_SIZE; ++i)
    {
        serializer.Write(true);
    }
    ASSERT_EQ(serializer.GetIndex(), BIG_DATA_SIZE);
}

TEST(FastDeserializerDeathTest, EnsureSizeFail)
{
    FastDeserializer deserializer("\0\0", 2);
\
    ASSERT_DEATH(
    {
        bool value;
        deserializer.Read(&value);
        deserializer.Read(&value);
        deserializer.Read(&value);
    }, "FastDeserializer: EnsureSize fail!");
}

TEST(FastDeserializer, Constructor)
{
    {
        FastDeserializer deserializer(nullptr, 0);
        EXPECT_TRUE(deserializer.IsEnd());
    }
    {
        FastDeserializer deserializer(nullptr, 1);
        EXPECT_FALSE(deserializer.IsEnd());
    }
}

TEST(FastDeserializer, ReadBool)
{
    const char* const DATA = "\0134gkfo\0\01";
    const int DATA_SIZE = 11;

    FastDeserializer deserializer(DATA, DATA_SIZE);

    for (int i = 0; i < DATA_SIZE; ++i)
    {
        ASSERT_FALSE(deserializer.IsEnd());
        bool value;
        deserializer.Read(&value);
        if (DATA[i] == '\0')
        {
            EXPECT_FALSE(value);
        }
        else
        {
            EXPECT_TRUE(value);
        }
    }

    ASSERT_TRUE(deserializer.IsEnd());
}

TEST(FastDeserializer, ReadInt32)
{
    const char* const DATA =
        "\x00\x00\x00\x00"
        "\x01\x00\x00\x00"
        "\xFF\x00\x00\x00"
        "\x47\xA3\x0B\x7A"
        "\xFF\xFF\xFF\xFF"
        "\xFF\x1F\xFF\xFF";
    const int DATA_SIZE = 24;

    FastDeserializer deserializer(DATA, DATA_SIZE);
    qint32 value;

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 0);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 1);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 255);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 2047583047);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, -1);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, -57345);

    ASSERT_TRUE(deserializer.IsEnd());
}

TEST(FastDeserializer, ReadUint32)
{
    const char* const DATA =
        "\x00\x00\x00\x00"
        "\x01\x00\x00\x00"
        "\xFF\x00\x00\x00"
        "\x47\xA3\x0B\x7A"
        "\xFF\xFF\xFF\xFF"
        "\xFF\x1F\xFF\xFF";
    const int DATA_SIZE = 24;

    FastDeserializer deserializer(DATA, DATA_SIZE);
    quint32 value;

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 0);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 1);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 255);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 2047583047);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 4294967295);

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, 4294909951);

    ASSERT_TRUE(deserializer.IsEnd());
}

TEST(FastDeserializer, ReadString)
{
    const char* const DATA =
        "\x00\x00\x00\x00"
        "\x03\x00\x00\x00"
        "\x48\x00\x65\x00\x20\x00"
        "\x05\x00\x00\x00"
        "\x1F\x04\x20\x00\x20\x00\x3C\x04\x20\x00";
    const int DATA_SIZE = 28;

    FastDeserializer deserializer(DATA, DATA_SIZE);
    QString value;

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, "");

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, "He ");

    ASSERT_FALSE(deserializer.IsEnd());
    deserializer.Read(&value);
    EXPECT_EQ(value, "П  м ");

    ASSERT_TRUE(deserializer.IsEnd());
}

TEST(FastSerializeDeserialize, VariousValues)
{
    {
        FastSerializer serializer;
        serializer.Write(true);
        serializer.Write(false);
        serializer.Write(42);
        serializer.Write(-14329);
        serializer.Write(QString("One two three"));
        serializer.Write(QString("Ехал грека через реку"));
        serializer.Write(4294967294u);

        FastDeserializer deserializer(
            serializer.GetData(),
            serializer.GetIndex());

        qDebug() << serializer.GetIndex();

        {
            bool value;

            deserializer.Read(&value);
            EXPECT_EQ(value, true);
            deserializer.Read(&value);
            EXPECT_EQ(value, false);
        }
        {
            int value;

            deserializer.Read(&value);
            EXPECT_EQ(value, 42);
            deserializer.Read(&value);
            EXPECT_EQ(value, -14329);
        }
        {
            QString value;

            deserializer.Read(&value);
            EXPECT_EQ(value, "One two three");
            deserializer.Read(&value);
            EXPECT_EQ(value, "Ехал грека через реку");
        }
        {
            unsigned int value;

            deserializer.Read(&value);
            EXPECT_EQ(value, 4294967294u);
        }

        ASSERT_TRUE(deserializer.IsEnd());
    }
}
