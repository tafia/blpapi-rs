BIN = \
	ConnectionAndAuthExample.exe \
	ContributionsMktdataExample.exe \
    ContributionsPageExample.exe \
    CorrelationExample.exe \
    EntitlementsVerificationExample.exe \
    EntitlementsVerificationSubscriptionExample.exe \
    EntitlementsVerificationSubscriptionTokenExample.exe \
    EntitlementsVerificationTokenExample.exe \
    GenerateTokenExample.exe \
    GenerateTokenSubscriptionExample.exe \
    IntradayBarExample.exe \
    IntradayTickExample.exe \
    LocalMktdataSubscriptionExample.exe \
    LocalPageSubscriptionExample.exe \
    MktdataBroadcastPublisherExample.exe \
    MktdataPublisher.exe \
    PagePublisherExample.exe \
    RefDataExample.exe \
    RefDataTableOverrideExample.exe \
    RequestServiceExample.exe \
    SecurityLookupExample.exe \
    SimpleBlockingRequestExample.exe \
    SimpleCategorizedFieldSearchExample.exe \
    SimpleFieldInfoExample.exe \
    SimpleFieldSearchExample.exe \
    SimpleHistoryExample.exe \
    SimpleIntradayBarExample.exe \
    SimpleIntradayTickExample.exe \
    SimpleRefDataExample.exe \
    SimpleRefDataOverrideExample.exe \
    SimpleSubscriptionExample.exe \
    SimpleSubscriptionIntervalExample.exe \
    SnapshotRequestTemplateExample.exe \
    SubscriptionCorrelationExample.exe \
    SubscriptionWithEventHandlerExample.exe \
    UserModeExample.exe

LFLAGS = /EHsc /O2 /D WIN32 /I..\include
CPPFLAGS = $(LFLAGS) ws2_32.lib ..\lib\blpapi3_64.lib

all: $(BIN)

clean:
	-@erase *.obj $(BIN)
