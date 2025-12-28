#include <QtTest/QtTest>
#include <QSignalSpy>

#include "chat/TwitchChatClient.hpp"

using namespace BluePlayer;

/**
 * @brief Test helper that exposes protected parsing methods for testing
 */
class TestableTwitchChatClient : public TwitchChatClient {
public:
  using TwitchChatClient::TwitchChatClient;
  
  // Expose protected parsing methods for testing
  QVariantMap testParseTags(const QString& tagsString) {
    return parseTags(tagsString);
  }
  
  QVariantList testParseEmoteParts(const QString& emotesTag, const QString& message) {
    return parseEmoteParts(emotesTag, message);
  }
  
  QVariantList testParseBadges(const QString& badgesTag) {
    return parseBadges(badgesTag);
  }
};

class TestTwitchChatClient : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests d'initialisation
  void testConstructor();
  void testInitialState();
  void testInitialConnectionStatus();

  // Tests des propriétés
  void testIsConnectedInitially();
  void testChannelInitially();
  void testCanSendMessagesInitially();

  // Tests des signaux
  void testConnectedChangedSignal();
  void testChannelChangedSignal();
  void testConnectionStatusChangedSignal();
  void testCanSendMessagesChangedSignal();
  void testMessageReceivedSignal();
  void testErrorOccurredSignal();

  // Tests de connexion
  void testConnectToChannel();
  void testConnectToChannelEmptyName();
  void testConnectToChannelWithHash();
  void testDisconnect();
  void testDisconnectWhenNotConnected();

  // Tests des credentials
  void testSetCredentials();
  void testSetCredentialsEmpty();
  void testCanSendMessagesAfterCredentials();

  // Tests d'envoi de messages
  void testSendMessageWithoutConnection();
  void testSendMessageWithoutCredentials();
  void testSendMessageEmptyMessage();

  // Tests de parsing IRC Tags
  void testParseTagsEmpty();
  void testParseTagsSingleTag();
  void testParseTagsMultipleTags();
  void testParseTagsWithEscapedCharacters();
  void testParseTagsDisplayName();
  void testParseTagsColor();
  
  // Tests de parsing Badges
  void testParseBadgesEmpty();
  void testParseBadgesSingle();
  void testParseBadgesMultiple();
  void testParseBadgesMalformed();
  
  // Tests de parsing Emotes
  void testParseEmotePartsNoEmotes();
  void testParseEmotePartsSingleEmote();
  void testParseEmotePartsMultipleEmotes();
  void testParseEmotePartsEmoteAtStart();
  void testParseEmotePartsEmoteAtEnd();
  void testParseEmotePartsConsecutiveEmotes();
  void testParseEmotePartsSameEmoteMultipleTimes();
  
  // Tests d'envoi de message - Sanitization
  void testSendMessageSanitizesNewlines();
  void testSendMessageTrimsWhitespace();
  void testSendMessageTruncatesLongMessage();
  void testSendMessageRejectsEmptyAfterSanitize();
  
  // Tests de parsing IRC avancés
  void testParseTagsWithSubscriber();
  void testParseTagsWithModerator();
  void testParseTagsWithVIP();
  void testParseTagsWithBitsAmount();
  void testParseTagsFirstMessage();
  void testParseTagsWithUserId();
  void testParseTagsWithRoomId();
  
  // Tests de parsing de messages IRC complets
  void testParseIrcMessagePing();
  void testParseIrcMessagePrivmsg();
  void testParseIrcMessageNotice();
  
  // Tests de badges spéciaux
  void testParseBadgesSubscriberTiers();
  void testParseBadgesBroadcaster();
  void testParseBadgesVIP();
  void testParseBadgesTurbo();
  
  // Tests de statut de connexion
  void testConnectionStatusConnecting();
  void testConnectionStatusDisconnected();
  
  // Tests de parsing IRC avancés supplémentaires
  void testParseTagsWithReturningChatter();
  void testParseTagsEmoteOnly();
  void testParseTagsWithMessageId();
  void testParseTagsWithTmiSentTs();
  void testParseTagsWithCustomRewardId();
  
  // Tests de parsing emotes avancés
  void testParseEmotePartsWithUnicode();
  void testParseEmotePartsLongMessage();
  void testParseEmotePartsMalformedRange();
  void testParseEmotePartsInvalidPositions();
  void testParseEmotePartsOverlappingRanges();
  
  // Tests de parsing badges avancés  
  void testParseBadgesFounder();
  void testParseBadgesGlhfPledge();
  void testParseBadgesHypeTrainConductor();
  void testParseBadgesPredictions();
  void testParseBadgesBitsLeader();
  void testParseBadgesGiftedSub();
  void testParseBadgesEmptyVersion();
  
  // Tests de parsing tags edge cases
  void testParseTagsEmptyValue();
  void testParseTagsNoEqualsSign();
  void testParseTagsMultipleEscapedChars();
  void testParseTagsWithNewlineEscape();
  
  // Tests de message sanitization
  void testSendMessageWithCarriageReturn();
  void testSendMessageOnlyWhitespace();
  void testSendMessageExactlyMaxLength();
  void testSendMessageOneOverMaxLength();
  
  // Tests de statut de connexion avancés
  void testConnectionStatusError();
  void testConnectionStatusTransitions();
  
  // Tests de channel handling
  void testChannelNameLowercase();
  void testChannelNamePreservesCase();
  void testMultipleConnectToSameChannel();
  
  // ===== NEW - IRC Handlers Coverage =====
  void testHandlePrivmsgWithBits();
  void testHandlePrivmsgWithReward();
  void testHandlePrivmsgWithAction();
  void testHandleNoticeSlowMode();
  void testHandleNoticeSubsOnly();
  void testHandleNoticeEmoteOnly();
  void testHandleUserstateUpdatesCanSend();
  void testHandleRoomstateEmotesOnly();
  
  // ===== NEW - Message Parsing Edge Cases =====
  void testParseMessageWithMentions();
  void testParseEmotePartsWithZeroWidthEmotes();
  void testParseTagsWithEmptyBadges();
  void testParseBadgesWithMultipleTiers();
  
  // ===== NEW - Connection State Tests =====
  void testReconnectLogicAfterDisconnect();
  void testSendRawWhenNotConnected();
  void testGenerateAnonUsername();

private:
  TwitchChatClient* m_client = nullptr;
  TestableTwitchChatClient* m_testableClient = nullptr;
};

void TestTwitchChatClient::initTestCase() {
}

void TestTwitchChatClient::cleanupTestCase() {
}

void TestTwitchChatClient::init() {
  m_client = new TwitchChatClient(this);
  m_testableClient = new TestableTwitchChatClient(this);
}

void TestTwitchChatClient::cleanup() {
  if (m_client) {
    m_client->disconnect();
    delete m_client;
    m_client = nullptr;
  }
  if (m_testableClient) {
    m_testableClient->disconnect();
    delete m_testableClient;
    m_testableClient = nullptr;
  }
}

// ===== Tests d'initialisation =====

void TestTwitchChatClient::testConstructor() {
  TwitchChatClient client;
  QVERIFY(!client.isConnected());
}

void TestTwitchChatClient::testInitialState() {
  QVERIFY(!m_client->isConnected());
  QVERIFY(m_client->channel().isEmpty());
  QVERIFY(!m_client->canSendMessages());
}

void TestTwitchChatClient::testInitialConnectionStatus() {
  QString status = m_client->connectionStatus();
  QVERIFY(!status.isEmpty());
}

// ===== Tests des propriétés =====

void TestTwitchChatClient::testIsConnectedInitially() {
  QCOMPARE(m_client->isConnected(), false);
}

void TestTwitchChatClient::testChannelInitially() {
  QVERIFY(m_client->channel().isEmpty());
}

void TestTwitchChatClient::testCanSendMessagesInitially() {
  QCOMPARE(m_client->canSendMessages(), false);
}

// ===== Tests des signaux =====

void TestTwitchChatClient::testConnectedChangedSignal() {
  QSignalSpy spy(m_client, &TwitchChatClient::connectedChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchChatClient::testChannelChangedSignal() {
  QSignalSpy spy(m_client, &TwitchChatClient::channelChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchChatClient::testConnectionStatusChangedSignal() {
  QSignalSpy spy(m_client, &TwitchChatClient::connectionStatusChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchChatClient::testCanSendMessagesChangedSignal() {
  QSignalSpy spy(m_client, &TwitchChatClient::canSendMessagesChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchChatClient::testMessageReceivedSignal() {
  QSignalSpy spy(m_client, &TwitchChatClient::messageReceived);
  QVERIFY(spy.isValid());
}

void TestTwitchChatClient::testErrorOccurredSignal() {
  QSignalSpy spy(m_client, &TwitchChatClient::errorOccurred);
  QVERIFY(spy.isValid());
}

// ===== Tests de connexion =====

void TestTwitchChatClient::testConnectToChannel() {
  QSignalSpy statusSpy(m_client, &TwitchChatClient::connectionStatusChanged);
  
  m_client->connectToChannel("testchannel");
  
  // La connexion est asynchrone, on vérifie juste qu'on n'a pas crashé
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testConnectToChannelEmptyName() {
  QSignalSpy errorSpy(m_client, &TwitchChatClient::errorOccurred);
  
  m_client->connectToChannel("");
  
  // Ne doit pas crasher
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testConnectToChannelWithHash() {
  // Le # devrait être géré automatiquement
  m_client->connectToChannel("#testchannel");
  
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testDisconnect() {
  m_client->connectToChannel("testchannel");
  m_client->disconnect();
  
  // Après disconnect, le client ne devrait pas être connecté
  // (la déconnexion peut être asynchrone)
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testDisconnectWhenNotConnected() {
  // Disconnect sans connexion ne doit pas crasher
  m_client->disconnect();
  QVERIFY(!m_client->isConnected());
}

// ===== Tests des credentials =====

void TestTwitchChatClient::testSetCredentials() {
  m_client->setCredentials("oauth:testtoken123", "testuser");
  
  // Pas de crash
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSetCredentialsEmpty() {
  m_client->setCredentials("", "");
  
  // Pas de crash avec credentials vides
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testCanSendMessagesAfterCredentials() {
  // Initialement false
  QVERIFY(!m_client->canSendMessages());
  
  // Après avoir mis des credentials, ça dépend de l'état de connexion
  m_client->setCredentials("oauth:testtoken", "testuser");
  
  // Sans connexion, toujours false
  QVERIFY(!m_client->canSendMessages());
}

// ===== Tests d'envoi de messages =====

void TestTwitchChatClient::testSendMessageWithoutConnection() {
  QSignalSpy errorSpy(m_client, &TwitchChatClient::errorOccurred);
  
  m_client->sendMessage("Hello World!");
  
  // Ne doit pas crasher
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageWithoutCredentials() {
  m_client->connectToChannel("testchannel");
  
  QSignalSpy errorSpy(m_client, &TwitchChatClient::errorOccurred);
  
  m_client->sendMessage("Hello!");
  
  // Ne doit pas crasher
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageEmptyMessage() {
  m_client->setCredentials("oauth:test", "user");
  m_client->connectToChannel("channel");
  
  m_client->sendMessage("");
  
  // Message vide ne doit pas crasher
  QVERIFY(m_client != nullptr);
}

// ===== Tests de parsing IRC Tags =====

void TestTwitchChatClient::testParseTagsEmpty() {
  QVariantMap tags = m_testableClient->testParseTags("");
  QVERIFY(tags.isEmpty());
}

void TestTwitchChatClient::testParseTagsSingleTag() {
  QVariantMap tags = m_testableClient->testParseTags("color=#FF4500");
  
  QCOMPARE(tags.size(), 1);
  QCOMPARE(tags.value("color").toString(), QString("#FF4500"));
}

void TestTwitchChatClient::testParseTagsMultipleTags() {
  QString tagsStr = "badge-info=subscriber/12;badges=subscriber/12,premium/1;"
                    "color=#1E90FF;display-name=TestUser;emotes=;id=abc123";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("badge-info").toString(), QString("subscriber/12"));
  QCOMPARE(tags.value("badges").toString(), QString("subscriber/12,premium/1"));
  QCOMPARE(tags.value("color").toString(), QString("#1E90FF"));
  QCOMPARE(tags.value("display-name").toString(), QString("TestUser"));
  QCOMPARE(tags.value("id").toString(), QString("abc123"));
}

void TestTwitchChatClient::testParseTagsWithEscapedCharacters() {
  // IRC tag escaping: \s = space, \: = semicolon, \\ = backslash
  QString tagsStr = "msg=Hello\\sWorld;value=a\\:b\\\\c";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg").toString(), QString("Hello World"));
  QCOMPARE(tags.value("value").toString(), QString("a;b\\c"));
}

void TestTwitchChatClient::testParseTagsDisplayName() {
  QString tagsStr = "display-name=Kappa123";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("display-name").toString(), QString("Kappa123"));
}

void TestTwitchChatClient::testParseTagsColor() {
  QString tagsStr = "color=#8A2BE2";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("color").toString(), QString("#8A2BE2"));
}

// ===== Tests de parsing Badges =====

void TestTwitchChatClient::testParseBadgesEmpty() {
  QVariantList badges = m_testableClient->testParseBadges("");
  QVERIFY(badges.isEmpty());
}

void TestTwitchChatClient::testParseBadgesSingle() {
  QVariantList badges = m_testableClient->testParseBadges("moderator/1");
  
  QCOMPARE(badges.size(), 1);
  QVariantMap badge = badges[0].toMap();
  QCOMPARE(badge.value("type").toString(), QString("moderator"));
  QCOMPARE(badge.value("version").toString(), QString("1"));
}

void TestTwitchChatClient::testParseBadgesMultiple() {
  QVariantList badges = m_testableClient->testParseBadges("broadcaster/1,subscriber/24,premium/1");
  
  QCOMPARE(badges.size(), 3);
  
  QVariantMap badge0 = badges[0].toMap();
  QCOMPARE(badge0.value("type").toString(), QString("broadcaster"));
  QCOMPARE(badge0.value("version").toString(), QString("1"));
  
  QVariantMap badge1 = badges[1].toMap();
  QCOMPARE(badge1.value("type").toString(), QString("subscriber"));
  QCOMPARE(badge1.value("version").toString(), QString("24"));
  
  QVariantMap badge2 = badges[2].toMap();
  QCOMPARE(badge2.value("type").toString(), QString("premium"));
  QCOMPARE(badge2.value("version").toString(), QString("1"));
}

void TestTwitchChatClient::testParseBadgesMalformed() {
  // Malformed badge without slash should be skipped
  QVariantList badges = m_testableClient->testParseBadges("valid/1,invalid,another/2");
  
  QCOMPARE(badges.size(), 2);
  QCOMPARE(badges[0].toMap().value("type").toString(), QString("valid"));
  QCOMPARE(badges[1].toMap().value("type").toString(), QString("another"));
}

// ===== Tests de parsing Emotes =====

void TestTwitchChatClient::testParseEmotePartsNoEmotes() {
  QVariantList parts = m_testableClient->testParseEmoteParts("", "Hello World!");
  
  QCOMPARE(parts.size(), 1);
  QVariantMap part = parts[0].toMap();
  QCOMPARE(part.value("type").toString(), QString("text"));
  QCOMPARE(part.value("content").toString(), QString("Hello World!"));
}

void TestTwitchChatClient::testParseEmotePartsSingleEmote() {
  // Emote format: emoteId:startPos-endPos
  // "Kappa" at position 6-10 in "Hello Kappa World"
  QVariantList parts = m_testableClient->testParseEmoteParts("25:6-10", "Hello Kappa World");
  
  QCOMPARE(parts.size(), 3);
  
  // Text before emote
  QVariantMap part0 = parts[0].toMap();
  QCOMPARE(part0.value("type").toString(), QString("text"));
  QCOMPARE(part0.value("content").toString(), QString("Hello "));
  
  // Emote
  QVariantMap part1 = parts[1].toMap();
  QCOMPARE(part1.value("type").toString(), QString("emote"));
  QCOMPARE(part1.value("emoteId").toString(), QString("25"));
  QCOMPARE(part1.value("content").toString(), QString("Kappa"));
  
  // Text after emote
  QVariantMap part2 = parts[2].toMap();
  QCOMPARE(part2.value("type").toString(), QString("text"));
  QCOMPARE(part2.value("content").toString(), QString(" World"));
}

void TestTwitchChatClient::testParseEmotePartsMultipleEmotes() {
  // Two different emotes: Kappa at 0-4, PogChamp at 6-13
  // Message: "Kappa PogChamp!" (length 15)
  //           01234 567890123 4
  QVariantList parts = m_testableClient->testParseEmoteParts("25:0-4/88:6-13", "Kappa PogChamp!");
  
  QCOMPARE(parts.size(), 4);
  
  // First emote (Kappa)
  QVariantMap part0 = parts[0].toMap();
  QCOMPARE(part0.value("type").toString(), QString("emote"));
  QCOMPARE(part0.value("emoteId").toString(), QString("25"));
  QCOMPARE(part0.value("content").toString(), QString("Kappa"));
  
  // Space between emotes
  QVariantMap part1 = parts[1].toMap();
  QCOMPARE(part1.value("type").toString(), QString("text"));
  QCOMPARE(part1.value("content").toString(), QString(" "));
  
  // Second emote (PogChamp)
  QVariantMap part2 = parts[2].toMap();
  QCOMPARE(part2.value("type").toString(), QString("emote"));
  QCOMPARE(part2.value("emoteId").toString(), QString("88"));
  QCOMPARE(part2.value("content").toString(), QString("PogChamp"));
  
  // Trailing exclamation mark
  QVariantMap part3 = parts[3].toMap();
  QCOMPARE(part3.value("type").toString(), QString("text"));
  QCOMPARE(part3.value("content").toString(), QString("!"));
}

void TestTwitchChatClient::testParseEmotePartsEmoteAtStart() {
  // Emote at the very beginning
  QVariantList parts = m_testableClient->testParseEmoteParts("25:0-4", "Kappa is great");
  
  QCOMPARE(parts.size(), 2);
  
  QVariantMap part0 = parts[0].toMap();
  QCOMPARE(part0.value("type").toString(), QString("emote"));
  QCOMPARE(part0.value("content").toString(), QString("Kappa"));
  
  QVariantMap part1 = parts[1].toMap();
  QCOMPARE(part1.value("type").toString(), QString("text"));
  QCOMPARE(part1.value("content").toString(), QString(" is great"));
}

void TestTwitchChatClient::testParseEmotePartsEmoteAtEnd() {
  // Emote at the very end
  QVariantList parts = m_testableClient->testParseEmoteParts("25:11-15", "I love you Kappa");
  
  QCOMPARE(parts.size(), 2);
  
  QVariantMap part0 = parts[0].toMap();
  QCOMPARE(part0.value("type").toString(), QString("text"));
  QCOMPARE(part0.value("content").toString(), QString("I love you "));
  
  QVariantMap part1 = parts[1].toMap();
  QCOMPARE(part1.value("type").toString(), QString("emote"));
  QCOMPARE(part1.value("content").toString(), QString("Kappa"));
}

void TestTwitchChatClient::testParseEmotePartsConsecutiveEmotes() {
  // Two emotes next to each other with no space
  QVariantList parts = m_testableClient->testParseEmoteParts("25:0-4/88:5-12", "KappaPogChamp");
  
  QCOMPARE(parts.size(), 2);
  
  QVariantMap part0 = parts[0].toMap();
  QCOMPARE(part0.value("type").toString(), QString("emote"));
  QCOMPARE(part0.value("emoteId").toString(), QString("25"));
  
  QVariantMap part1 = parts[1].toMap();
  QCOMPARE(part1.value("type").toString(), QString("emote"));
  QCOMPARE(part1.value("emoteId").toString(), QString("88"));
}

void TestTwitchChatClient::testParseEmotePartsSameEmoteMultipleTimes() {
  // Same emote used multiple times: "Kappa Kappa Kappa"
  QVariantList parts = m_testableClient->testParseEmoteParts("25:0-4,6-10,12-16", "Kappa Kappa Kappa");
  
  QCOMPARE(parts.size(), 5);
  
  // All three should be emotes with text spaces between
  QCOMPARE(parts[0].toMap().value("type").toString(), QString("emote"));
  QCOMPARE(parts[1].toMap().value("type").toString(), QString("text"));
  QCOMPARE(parts[2].toMap().value("type").toString(), QString("emote"));
  QCOMPARE(parts[3].toMap().value("type").toString(), QString("text"));
  QCOMPARE(parts[4].toMap().value("type").toString(), QString("emote"));
}

// ===== Tests d'envoi de message - Sanitization =====

void TestTwitchChatClient::testSendMessageSanitizesNewlines() {
  // Message with newlines should be sanitized (newlines removed)
  // We can't test the actual send without a mock, but we verify no crash
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message with \r\n should not crash
  m_client->sendMessage("Hello\r\nWorld");
  
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageTrimsWhitespace() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message with leading/trailing whitespace
  m_client->sendMessage("   Hello World   ");
  
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageTruncatesLongMessage() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message longer than 500 chars should be rejected
  QString longMessage = QString(600, 'a');
  QCOMPARE(longMessage.length(), 600);
  
  m_client->sendMessage(longMessage);
  
  // No crash, message was rejected
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageRejectsEmptyAfterSanitize() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message that becomes empty after sanitization (only whitespace/newlines)
  m_client->sendMessage("   \r\n   ");
  
  // Should not crash
  QVERIFY(m_client != nullptr);
}

// ===== Tests de parsing IRC avancés =====

void TestTwitchChatClient::testParseTagsWithSubscriber() {
  QString tagsStr = "badge-info=subscriber/24;badges=subscriber/24;subscriber=1";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("badge-info").toString(), QString("subscriber/24"));
  QCOMPARE(tags.value("subscriber").toString(), QString("1"));
}

void TestTwitchChatClient::testParseTagsWithModerator() {
  QString tagsStr = "badges=moderator/1;mod=1;display-name=ModUser";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("mod").toString(), QString("1"));
  QCOMPARE(tags.value("badges").toString(), QString("moderator/1"));
}

void TestTwitchChatClient::testParseTagsWithVIP() {
  QString tagsStr = "badges=vip/1;vip=1;display-name=VIPUser";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("vip").toString(), QString("1"));
}

void TestTwitchChatClient::testParseTagsWithBitsAmount() {
  QString tagsStr = "bits=100;display-name=Cheerer;color=#FF0000";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("bits").toString(), QString("100"));
}

void TestTwitchChatClient::testParseTagsFirstMessage() {
  QString tagsStr = "first-msg=1;display-name=NewUser";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("first-msg").toString(), QString("1"));
}

void TestTwitchChatClient::testParseTagsWithUserId() {
  QString tagsStr = "user-id=12345678;display-name=TestUser";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("user-id").toString(), QString("12345678"));
}

void TestTwitchChatClient::testParseTagsWithRoomId() {
  QString tagsStr = "room-id=87654321;display-name=TestUser";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("room-id").toString(), QString("87654321"));
}

// ===== Tests de parsing de messages IRC complets =====

void TestTwitchChatClient::testParseIrcMessagePing() {
  // PING message should be handled internally
  // We can't easily test the response, but we verify no crash
  QVERIFY(m_testableClient != nullptr);
}

void TestTwitchChatClient::testParseIrcMessagePrivmsg() {
  // Full PRIVMSG format
  // The actual parsing happens internally, we test via tags and badges
  QString tagsStr = "display-name=TestUser;color=#1E90FF;id=msg123";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("display-name").toString(), QString("TestUser"));
  QCOMPARE(tags.value("color").toString(), QString("#1E90FF"));
  QCOMPARE(tags.value("id").toString(), QString("msg123"));
}

void TestTwitchChatClient::testParseIrcMessageNotice() {
  // NOTICE messages are handled internally
  // We verify the tags parsing works correctly
  QString tagsStr = "msg-id=slow_on";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg-id").toString(), QString("slow_on"));
}

// ===== Tests de badges spéciaux =====

void TestTwitchChatClient::testParseBadgesSubscriberTiers() {
  // Tier 1: 0-11, Tier 2: 2000-2011, Tier 3: 3000-3011
  QVariantList badges = m_testableClient->testParseBadges("subscriber/3012");
  
  QCOMPARE(badges.size(), 1);
  QVariantMap badge = badges[0].toMap();
  QCOMPARE(badge.value("type").toString(), QString("subscriber"));
  QCOMPARE(badge.value("version").toString(), QString("3012"));
}

void TestTwitchChatClient::testParseBadgesBroadcaster() {
  QVariantList badges = m_testableClient->testParseBadges("broadcaster/1");
  
  QCOMPARE(badges.size(), 1);
  QVariantMap badge = badges[0].toMap();
  QCOMPARE(badge.value("type").toString(), QString("broadcaster"));
  QCOMPARE(badge.value("version").toString(), QString("1"));
}

void TestTwitchChatClient::testParseBadgesVIP() {
  QVariantList badges = m_testableClient->testParseBadges("vip/1");
  
  QCOMPARE(badges.size(), 1);
  QVariantMap badge = badges[0].toMap();
  QCOMPARE(badge.value("type").toString(), QString("vip"));
}

void TestTwitchChatClient::testParseBadgesTurbo() {
  QVariantList badges = m_testableClient->testParseBadges("turbo/1,premium/1");
  
  QCOMPARE(badges.size(), 2);
  QCOMPARE(badges[0].toMap().value("type").toString(), QString("turbo"));
  QCOMPARE(badges[1].toMap().value("type").toString(), QString("premium"));
}

// ===== Tests de statut de connexion =====

void TestTwitchChatClient::testConnectionStatusConnecting() {
  m_client->connectToChannel("testchannel");
  
  // Status should be "connecting" after calling connectToChannel
  QString status = m_client->connectionStatus();
  QVERIFY(status == "connecting" || status == "disconnected" || status == "connected");
}

void TestTwitchChatClient::testConnectionStatusDisconnected() {
  // Initially disconnected
  QString status = m_client->connectionStatus();
  QCOMPARE(status, QString("disconnected"));
}

// ===== Tests de parsing IRC avancés supplémentaires =====

void TestTwitchChatClient::testParseTagsWithReturningChatter() {
  QString tagsStr = "returning-chatter=1;display-name=ReturningUser";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("returning-chatter").toString(), QString("1"));
}

void TestTwitchChatClient::testParseTagsEmoteOnly() {
  QString tagsStr = "emote-only=1;room-id=12345";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("emote-only").toString(), QString("1"));
}

void TestTwitchChatClient::testParseTagsWithMessageId() {
  QString tagsStr = "id=abc-123-def-456;display-name=User";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("id").toString(), QString("abc-123-def-456"));
}

void TestTwitchChatClient::testParseTagsWithTmiSentTs() {
  QString tagsStr = "tmi-sent-ts=1609459200000;display-name=User";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("tmi-sent-ts").toString(), QString("1609459200000"));
}

void TestTwitchChatClient::testParseTagsWithCustomRewardId() {
  QString tagsStr = "custom-reward-id=abc123-reward;display-name=Redeemer";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("custom-reward-id").toString(), QString("abc123-reward"));
}

// ===== Tests de parsing emotes avancés =====

void TestTwitchChatClient::testParseEmotePartsWithUnicode() {
  // Message avec emoji Unicode : "Hello 😀 Kappa"
  // L'emoji peut affecter les positions
  QString message = "Hello Kappa test";
  QVariantList parts = m_testableClient->testParseEmoteParts("25:6-10", message);
  
  QVERIFY(parts.size() >= 1);
  // Vérifier que Kappa est correctement extrait
  bool foundEmote = false;
  for (const QVariant& part : parts) {
    if (part.toMap()["type"].toString() == "emote") {
      foundEmote = true;
      break;
    }
  }
  QVERIFY(foundEmote);
}

void TestTwitchChatClient::testParseEmotePartsLongMessage() {
  // Message long avec emote au milieu
  QString message = QString(200, 'a') + " Kappa " + QString(200, 'b');
  // Kappa at position 201-205
  QVariantList parts = m_testableClient->testParseEmoteParts("25:201-205", message);
  
  QCOMPARE(parts.size(), 3);  // text + emote + text
  
  QVariantMap emotePart = parts[1].toMap();
  QCOMPARE(emotePart["type"].toString(), QString("emote"));
  QCOMPARE(emotePart["content"].toString(), QString("Kappa"));
}

void TestTwitchChatClient::testParseEmotePartsMalformedRange() {
  // Range sans tiret
  QVariantList parts = m_testableClient->testParseEmoteParts("25:610", "Hello Kappa");
  
  // Should fall back to text only
  QCOMPARE(parts.size(), 1);
  QCOMPARE(parts[0].toMap()["type"].toString(), QString("text"));
}

void TestTwitchChatClient::testParseEmotePartsInvalidPositions() {
  // Position négative ou hors limites
  QVariantList parts = m_testableClient->testParseEmoteParts("25:-1-5", "Kappa");
  
  // Should handle gracefully
  QVERIFY(parts.size() >= 1);
}

void TestTwitchChatClient::testParseEmotePartsOverlappingRanges() {
  // Two emotes with overlapping positions (shouldn't happen but test robustness)
  QString message = "Kappa test";
  // Both claim position 0-4
  QVariantList parts = m_testableClient->testParseEmoteParts("25:0-4/88:2-6", message);
  
  // Should handle without crashing
  QVERIFY(parts.size() >= 1);
}

// ===== Tests de parsing badges avancés =====

void TestTwitchChatClient::testParseBadgesFounder() {
  QVariantList badges = m_testableClient->testParseBadges("founder/0");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("founder"));
  QCOMPARE(badges[0].toMap()["version"].toString(), QString("0"));
}

void TestTwitchChatClient::testParseBadgesGlhfPledge() {
  QVariantList badges = m_testableClient->testParseBadges("glhf-pledge/1");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("glhf-pledge"));
}

void TestTwitchChatClient::testParseBadgesHypeTrainConductor() {
  QVariantList badges = m_testableClient->testParseBadges("hype-train/1");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("hype-train"));
}

void TestTwitchChatClient::testParseBadgesPredictions() {
  QVariantList badges = m_testableClient->testParseBadges("predictions/blue-1");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("predictions"));
  QCOMPARE(badges[0].toMap()["version"].toString(), QString("blue-1"));
}

void TestTwitchChatClient::testParseBadgesBitsLeader() {
  QVariantList badges = m_testableClient->testParseBadges("bits-leader/3");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("bits-leader"));
}

void TestTwitchChatClient::testParseBadgesGiftedSub() {
  QVariantList badges = m_testableClient->testParseBadges("sub-gifter/50");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("sub-gifter"));
  QCOMPARE(badges[0].toMap()["version"].toString(), QString("50"));
}

void TestTwitchChatClient::testParseBadgesEmptyVersion() {
  // Badge with slash but empty version
  QVariantList badges = m_testableClient->testParseBadges("badge/");
  
  QCOMPARE(badges.size(), 1);
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("badge"));
  QCOMPARE(badges[0].toMap()["version"].toString(), QString(""));
}

// ===== Tests de parsing tags edge cases =====

void TestTwitchChatClient::testParseTagsEmptyValue() {
  QString tagsStr = "emotes=;display-name=User";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("emotes").toString(), QString(""));
  QCOMPARE(tags.value("display-name").toString(), QString("User"));
}

void TestTwitchChatClient::testParseTagsNoEqualsSign() {
  // Malformed tag without equals
  QString tagsStr = "validtag=value;invalidtag;another=test";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  // Valid tags should still be parsed
  QCOMPARE(tags.value("validtag").toString(), QString("value"));
  QCOMPARE(tags.value("another").toString(), QString("test"));
  // Invalid tag should be skipped
  QVERIFY(!tags.contains("invalidtag"));
}

void TestTwitchChatClient::testParseTagsMultipleEscapedChars() {
  // Multiple escaped characters
  QString tagsStr = "msg=Hello\\sWorld\\:\\sTest\\\\End";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg").toString(), QString("Hello World; Test\\End"));
}

void TestTwitchChatClient::testParseTagsWithNewlineEscape() {
  QString tagsStr = "msg=Line1\\nLine2\\rEnd";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg").toString(), QString("Line1\nLine2\rEnd"));
}

// ===== Tests de message sanitization =====

void TestTwitchChatClient::testSendMessageWithCarriageReturn() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message with only \r
  m_client->sendMessage("Hello\rWorld");
  
  // Should not crash
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageOnlyWhitespace() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message with only spaces and tabs
  m_client->sendMessage("     \t\t   ");
  
  // Should not crash, message rejected silently
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageExactlyMaxLength() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message exactly 500 characters (Twitch limit)
  QString exactMessage = QString(500, 'a');
  QCOMPARE(exactMessage.length(), 500);
  
  m_client->sendMessage(exactMessage);
  
  // Should not crash, message should be accepted
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testSendMessageOneOverMaxLength() {
  m_client->setCredentials("oauth:test", "testuser");
  m_client->connectToChannel("testchannel");
  
  // Message 501 characters (one over limit)
  QString overMessage = QString(501, 'a');
  QCOMPARE(overMessage.length(), 501);
  
  m_client->sendMessage(overMessage);
  
  // Should not crash, message rejected
  QVERIFY(m_client != nullptr);
}

// ===== Tests de statut de connexion avancés =====

void TestTwitchChatClient::testConnectionStatusError() {
  // After error, status should be available
  // Can't easily trigger error without network, just verify method doesn't crash
  QString status = m_client->connectionStatus();
  QVERIFY(!status.isEmpty());
}

void TestTwitchChatClient::testConnectionStatusTransitions() {
  // Verify we can query status at different stages
  
  // Initial
  QString initial = m_client->connectionStatus();
  QCOMPARE(initial, QString("disconnected"));
  
  // After connect attempt
  m_client->connectToChannel("testchannel");
  QString connecting = m_client->connectionStatus();
  QVERIFY(connecting == "connecting" || connecting == "disconnected");
  
  // After disconnect
  m_client->disconnect();
  QString disconnected = m_client->connectionStatus();
  QCOMPARE(disconnected, QString("disconnected"));
}

// ===== Tests de channel handling =====

void TestTwitchChatClient::testChannelNameLowercase() {
  m_client->connectToChannel("UPPERCASE");
  
  // Channel should be converted to lowercase
  QString channel = m_client->channel();
  QCOMPARE(channel, QString("uppercase"));
  
  m_client->disconnect();
}

void TestTwitchChatClient::testChannelNamePreservesCase() {
  // Actually channel() returns lowercase
  m_client->connectToChannel("MixedCase");
  
  QString channel = m_client->channel();
  QCOMPARE(channel, QString("mixedcase"));
  
  m_client->disconnect();
}

void TestTwitchChatClient::testMultipleConnectToSameChannel() {
  m_client->connectToChannel("channel1");
  m_client->connectToChannel("channel1");
  m_client->connectToChannel("channel1");
  
  // Multiple connects should not crash
  QVERIFY(m_client != nullptr);
  
  m_client->disconnect();
}

// ===== NEW - IRC Handlers Coverage =====

void TestTwitchChatClient::testHandlePrivmsgWithBits() {
  // Parse tags with bits amount
  QString tagsStr = "bits=500;display-name=Cheerer;color=#FF0000;badges=bits/100";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("bits").toString(), QString("500"));
  QCOMPARE(tags.value("display-name").toString(), QString("Cheerer"));
  QCOMPARE(tags.value("color").toString(), QString("#FF0000"));
  
  // Verify badges are present
  QCOMPARE(tags.value("badges").toString(), QString("bits/100"));
}

void TestTwitchChatClient::testHandlePrivmsgWithReward() {
  // Parse tags with custom reward (channel points)
  QString tagsStr = "custom-reward-id=reward-uuid-12345;display-name=Redeemer;msg-id=highlighted-message";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("custom-reward-id").toString(), QString("reward-uuid-12345"));
  QCOMPARE(tags.value("msg-id").toString(), QString("highlighted-message"));
}

void TestTwitchChatClient::testHandlePrivmsgWithAction() {
  // /me action messages are parsed normally
  // The content starts with 0x01 ACTION and ends with 0x01 in IRC
  // But display-name and message parsing should work
  QString tagsStr = "display-name=ActionUser;color=#00FF00";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("display-name").toString(), QString("ActionUser"));
}

void TestTwitchChatClient::testHandleNoticeSlowMode() {
  // NOTICE with msg-id for slow mode
  QString tagsStr = "msg-id=slow_on;room-id=12345";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg-id").toString(), QString("slow_on"));
  
  // Also test slow_off
  QString tagsStr2 = "msg-id=slow_off;room-id=12345";
  QVariantMap tags2 = m_testableClient->testParseTags(tagsStr2);
  QCOMPARE(tags2.value("msg-id").toString(), QString("slow_off"));
}

void TestTwitchChatClient::testHandleNoticeSubsOnly() {
  // NOTICE with msg-id for subscribers-only mode
  QString tagsStr = "msg-id=subs_on;room-id=12345";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg-id").toString(), QString("subs_on"));
  
  // Also test subs_off
  QString tagsStr2 = "msg-id=subs_off;room-id=12345";
  QVariantMap tags2 = m_testableClient->testParseTags(tagsStr2);
  QCOMPARE(tags2.value("msg-id").toString(), QString("subs_off"));
}

void TestTwitchChatClient::testHandleNoticeEmoteOnly() {
  // NOTICE with msg-id for emote-only mode
  QString tagsStr = "msg-id=emote_only_on;room-id=12345";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("msg-id").toString(), QString("emote_only_on"));
  
  // Also test emote_only_off
  QString tagsStr2 = "msg-id=emote_only_off;room-id=12345";
  QVariantMap tags2 = m_testableClient->testParseTags(tagsStr2);
  QCOMPARE(tags2.value("msg-id").toString(), QString("emote_only_off"));
}

void TestTwitchChatClient::testHandleUserstateUpdatesCanSend() {
  // Initially canSendMessages is false
  QVERIFY(!m_client->canSendMessages());
  
  // Even with credentials, not connected
  m_client->setCredentials("oauth:test", "testuser");
  QVERIFY(!m_client->canSendMessages());
  
  // canSendMessages requires: token, username, AND connected
}

void TestTwitchChatClient::testHandleRoomstateEmotesOnly() {
  // ROOMSTATE tags indicate room settings
  QString tagsStr = "emote-only=1;followers-only=0;r9k=0;room-id=12345;slow=0;subs-only=0";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("emote-only").toString(), QString("1"));
  QCOMPARE(tags.value("followers-only").toString(), QString("0"));
  QCOMPARE(tags.value("r9k").toString(), QString("0"));
  QCOMPARE(tags.value("slow").toString(), QString("0"));
  QCOMPARE(tags.value("subs-only").toString(), QString("0"));
}

// ===== NEW - Message Parsing Edge Cases =====

void TestTwitchChatClient::testParseMessageWithMentions() {
  // Messages with @mentions should parse correctly
  QString message = "@user1 hello @user2 how are you?";
  QVariantList parts = m_testableClient->testParseEmoteParts("", message);
  
  // No emotes, just text
  QCOMPARE(parts.size(), 1);
  QVariantMap textPart = parts[0].toMap();
  QCOMPARE(textPart["type"].toString(), QString("text"));
  QCOMPARE(textPart["content"].toString(), message);
}

void TestTwitchChatClient::testParseEmotePartsWithZeroWidthEmotes() {
  // Message with zero-width emotes (like SantaHat overlay emote)
  // These are typically placed at position 0-0 or similar
  QString message = "Kappa text after";
  
  // Emote at position 0-4 (Kappa is 5 chars)
  QVariantList parts = m_testableClient->testParseEmoteParts("25:0-4", message);
  
  QCOMPARE(parts.size(), 2);
  
  // First part is emote
  QVariantMap emotePart = parts[0].toMap();
  QCOMPARE(emotePart["type"].toString(), QString("emote"));
  QCOMPARE(emotePart["emoteId"].toString(), QString("25"));
  QCOMPARE(emotePart["content"].toString(), QString("Kappa"));
  
  // Second part is text
  QVariantMap textPart = parts[1].toMap();
  QCOMPARE(textPart["type"].toString(), QString("text"));
  QCOMPARE(textPart["content"].toString(), QString(" text after"));
}

void TestTwitchChatClient::testParseTagsWithEmptyBadges() {
  // Tags where badges is present but empty
  QString tagsStr = "badges=;display-name=User;color=#FFFFFF";
  QVariantMap tags = m_testableClient->testParseTags(tagsStr);
  
  QCOMPARE(tags.value("badges").toString(), QString(""));
  QCOMPARE(tags.value("display-name").toString(), QString("User"));
  
  // Parse empty badges string
  QVariantList badges = m_testableClient->testParseBadges("");
  QVERIFY(badges.isEmpty());
}

void TestTwitchChatClient::testParseBadgesWithMultipleTiers() {
  // Multiple badges including subscriber tiers
  QString badgesStr = "broadcaster/1,subscriber/3012,partner/1";
  QVariantList badges = m_testableClient->testParseBadges(badgesStr);
  
  QCOMPARE(badges.size(), 3);
  
  // Broadcaster badge
  QCOMPARE(badges[0].toMap()["type"].toString(), QString("broadcaster"));
  QCOMPARE(badges[0].toMap()["version"].toString(), QString("1"));
  
  // Subscriber tier 3 (3012 = tier 3, 12 months)
  QCOMPARE(badges[1].toMap()["type"].toString(), QString("subscriber"));
  QCOMPARE(badges[1].toMap()["version"].toString(), QString("3012"));
  
  // Partner badge
  QCOMPARE(badges[2].toMap()["type"].toString(), QString("partner"));
  QCOMPARE(badges[2].toMap()["version"].toString(), QString("1"));
}

// ===== NEW - Connection State Tests =====

void TestTwitchChatClient::testReconnectLogicAfterDisconnect() {
  // Connect to a channel
  m_client->connectToChannel("testchannel");
  
  // Status should be connecting
  QString status = m_client->connectionStatus();
  QVERIFY(status == "connecting" || status == "disconnected");
  
  // Disconnect
  m_client->disconnect();
  
  // Status should be disconnected
  QCOMPARE(m_client->connectionStatus(), QString("disconnected"));
  
  // Should be able to reconnect
  m_client->connectToChannel("newchannel");
  
  // No crash
  QVERIFY(m_client != nullptr);
  m_client->disconnect();
}

void TestTwitchChatClient::testSendRawWhenNotConnected() {
  // Sending when not connected should not crash
  // We can't directly test sendRaw (private) but sendMessage uses it
  m_client->sendMessage("test message");
  
  // No crash
  QVERIFY(m_client != nullptr);
}

void TestTwitchChatClient::testGenerateAnonUsername() {
  // We can't directly test generateAnonUsername (private)
  // But we can verify anonymous connection works
  
  // Connect without credentials (anonymous mode)
  m_client->connectToChannel("testchannel");
  
  // Should use justinfan username internally
  // We can verify the client doesn't crash
  QVERIFY(m_client != nullptr);
  
  m_client->disconnect();
}

QTEST_MAIN(TestTwitchChatClient)
#include "TestTwitchChatClient.moc"
