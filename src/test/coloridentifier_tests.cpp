// Copyright (c) 2020 Chaintope Inc.
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <test/test_tapyrus.h>
#include <map>
#include <primitives/transaction.h>
#include <script/script.h>
#include <coloridentifier.h>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(coloridentifier_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(coloridentifier_valid_unserialize)
{
    //type NONE
    ColorIdentifier c0;
    uint8_t str[32] = {};
    CDataStream ss0(ParseHex("00"), SER_NETWORK, INIT_PROTO_VERSION);
    ss0 >> c0;
    BOOST_CHECK_EQUAL(TokenToUint(c0.type), TokenToUint(TokenTypes::NONE));
    BOOST_CHECK(memcmp(&c0.payload[0], &str[0], 32) == 0);

    try {
        CDataStream ss00(ParseHex("0100"), SER_NETWORK, INIT_PROTO_VERSION);
        ss00 >> c0;
        BOOST_CHECK_MESSAGE(false, "We should have thrown");
    } catch (const std::ios_base::failure& e) {
    }

    CDataStream ss00(ParseHex("038282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508"), SER_NETWORK, INIT_PROTO_VERSION);
    ss00 >> c0;

    //type REISSUABLE
    std::vector<unsigned char> scriptVector(ParseHex("038282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508"));
    uint8_t scripthash[CSHA256::OUTPUT_SIZE];
    scriptVector.insert(scriptVector.begin(), 0x21);
    CSHA256().Write(scriptVector.data(), scriptVector.size()).Finalize(scripthash);
    
    ColorIdentifier c1;
    CDataStream ss1(ParseHex("01f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6"), SER_NETWORK, INIT_PROTO_VERSION);
    ss1 >> c1;
    BOOST_CHECK_EQUAL(HexStr(&scripthash[0], &scripthash[32]), "f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6");
    BOOST_CHECK_EQUAL(HexStr(&c1.payload[0], &c1.payload[32]), "f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6");
    BOOST_CHECK_EQUAL(TokenToUint(c1.type), TokenToUint(TokenTypes::REISSUABLE));
    BOOST_CHECK(memcmp(&c1.payload[0], &scripthash[0], 32) == 0);

    //type NON_REISSUABLE
    uint256 hashMalFix(ParseHex("485273f6703f038a234400edadb543eb44b4af5372e8b207990beebc386e7954"));
    COutPoint out(hashMalFix, 0);
    CDataStream s(SER_NETWORK, INIT_PROTO_VERSION);
    s << out;
    CSHA256().Write((const unsigned char *)s.data(), s.size()).Finalize(scripthash);

    ColorIdentifier c2;
    CDataStream ss2(ParseHex("029608951ee23595caa227e7668e39f9d3525a39e9dc30d7391f138576c07be84d"), SER_NETWORK, INIT_PROTO_VERSION);
    ss2 >> c2;
    BOOST_CHECK_EQUAL(HexStr(&scripthash[0], &scripthash[32]), "9608951ee23595caa227e7668e39f9d3525a39e9dc30d7391f138576c07be84d");
    BOOST_CHECK_EQUAL(HexStr(&c2.payload[0], &c2.payload[32], false), "9608951ee23595caa227e7668e39f9d3525a39e9dc30d7391f138576c07be84d");
    BOOST_CHECK_EQUAL(TokenToUint(c2.type), TokenToUint(TokenTypes::NON_REISSUABLE));
    BOOST_CHECK(memcmp(&c2.payload[0], &scripthash[0], 32) == 0);

}

BOOST_AUTO_TEST_CASE(coloridentifier_valid_serialize)
{
    //type REISSUABLE
    std::vector<unsigned char> scriptVector(ParseHex("038282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508"));
    ColorIdentifier c1(CScript() << scriptVector);
    CDataStream ss1(SER_NETWORK, INIT_PROTO_VERSION);
    ss1 << c1;
    BOOST_CHECK_EQUAL(HexStr(ss1.begin(), ss1.end(), false), "01f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6");

    //type NON_REISSUABLE
    uint256 hashMalFix(ParseHex("485273f6703f038a234400edadb543eb44b4af5372e8b207990beebc386e7954"));
    COutPoint out(hashMalFix, 0);
    ColorIdentifier c2(out, TokenTypes::NON_REISSUABLE);
    CDataStream ss2(SER_NETWORK, INIT_PROTO_VERSION);
    ss2 << c2;
    BOOST_CHECK_EQUAL(HexStr(ss2.begin(), ss2.end()), "029608951ee23595caa227e7668e39f9d3525a39e9dc30d7391f138576c07be84d");
}

BOOST_AUTO_TEST_CASE(coloridentifier_compare)
{
    //type REISSUABLE
    std::vector<unsigned char> scriptVector(ParseHex("038282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508"));
    ColorIdentifier c1(CScript() << scriptVector);

    uint8_t scripthash[CSHA256::OUTPUT_SIZE];
    scriptVector.insert(scriptVector.begin(), 0x21);
    CSHA256().Write(scriptVector.data(), scriptVector.size()).Finalize(scripthash);
    ColorIdentifier c2;
    c2.type = TokenTypes::REISSUABLE;
    memcpy(&c2.payload[0], &scripthash[0], 32);

    BOOST_CHECK_EQUAL(HexStr(&scripthash[0], &scripthash[32]), "f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6");
    BOOST_CHECK_EQUAL(HexStr(&c1.payload[0], &c1.payload[32]), "f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6");
    BOOST_CHECK_EQUAL(HexStr(&c2.payload[0], &c2.payload[32]), "f55efb77e5a0e37c16d8f3484024558241c215a57aa991533152813f111482f6");
    BOOST_CHECK(c1.operator==(c2));

    //type NON_REISSUABLE
    uint256 hashMalFix(ParseHex("485273f6703f038a234400edadb543eb44b4af5372e8b207990beebc386e7954"));
    COutPoint out(hashMalFix, 0);
    ColorIdentifier c3(out, TokenTypes::NON_REISSUABLE);

    ColorIdentifier c4;
    CDataStream s(SER_NETWORK, INIT_PROTO_VERSION);
    s << out;
    CSHA256().Write((const unsigned char *)s.data(), s.size()).Finalize(scripthash);
    c4.type = TokenTypes::NON_REISSUABLE;
    memcpy(&c4.payload[0], &scripthash[0], 32);

    BOOST_CHECK(c3.operator==(c4));

    BOOST_CHECK(!c1.operator==(c3));
    BOOST_CHECK(!c2.operator==(c4));
}

BOOST_AUTO_TEST_CASE(coloridentifier_map_compare)
{
    //type REISSUABLE
    std::vector<unsigned char> scriptVector(ParseHex("038282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508"));
    ColorIdentifier c1(CScript() << scriptVector);

    uint8_t scripthash[CSHA256::OUTPUT_SIZE];
    scriptVector.insert(scriptVector.begin(), 0x21);
    CSHA256().Write(scriptVector.data(), scriptVector.size()).Finalize(scripthash);
    ColorIdentifier c2;
    c2.type = TokenTypes::REISSUABLE;
    memcpy(&c2.payload[0], &scripthash[0], 32);

    BOOST_CHECK_EQUAL(c1 < c2, false);

    //type NON_REISSUABLE
    uint256 hashMalFix(ParseHex("485273f6703f038a234400edadb543eb44b4af5372e8b207990beebc386e7954"));
    COutPoint out(hashMalFix, 0);
    ColorIdentifier c3(out, TokenTypes::NON_REISSUABLE);

    ColorIdentifier c4;
    CDataStream s(SER_NETWORK, INIT_PROTO_VERSION);
    s << out;
    CSHA256().Write((const unsigned char *)s.data(), s.size()).Finalize(scripthash);
    c4.type = TokenTypes::NON_REISSUABLE;
    memcpy(&c4.payload[0], &scripthash[0], 32);
    BOOST_CHECK_EQUAL(c3 < c4, false);

    BOOST_CHECK_EQUAL(c1 < c3, true);
    BOOST_CHECK_EQUAL(c2 < c4, true);
}

BOOST_AUTO_TEST_SUITE_END()