#include <etcd/client_impl.hpp>
#include <userver/crypto/base64.hpp>
#include <userver/etcd/client.hpp>
#include <userver/etcd/settings.hpp>
#include <userver/formats/json/inline.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/utest/http_client.hpp>
#include <userver/utest/http_server_mock.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/async.hpp>

USERVER_NAMESPACE_BEGIN

namespace {

utest::HttpServerMock::HttpResponse EtcdRequestProcessor(const utest::HttpServerMock::HttpRequest& request) {
    static std::map<std::string, std::string> storage;

    EXPECT_EQ(request.method, clients::http::HttpMethod::kPost);
    const auto request_body = formats::json::FromString(request.body);
    formats::json::ValueBuilder response_body_value_builder;

    if (request.path == "/v3/kv/put") {
        const auto key = crypto::base64::Base64Decode(request_body["key"].As<std::string>());
        const auto value = crypto::base64::Base64Decode(request_body["value"].As<std::string>());
        storage[key] = value;
    } else if (request.path == "/v3/kv/range") {
        const auto key = crypto::base64::Base64Decode(request_body["key"].As<std::string>());
        const auto range_end = crypto::base64::Base64Decode(request_body["range_end"].As<std::string>());
        auto first_key = storage.lower_bound(key);
        const auto last_key = storage.upper_bound(range_end);

        response_body_value_builder["kvs"] = formats::json::MakeArray();
        while (first_key != last_key) {
            response_body_value_builder["kvs"].PushBack(formats::json::MakeObject(
                "key",
                crypto::base64::Base64Encode(first_key->first),
                "value",
                crypto::base64::Base64Encode(first_key->second)
            ));
            ++first_key;
        }
    } else if (request.path == "/v3/kv/deleterange") {
        const auto key = crypto::base64::Base64Decode(request_body["key"].As<std::string>());
        storage.erase(key);
    }

    return utest::HttpServerMock::HttpResponse{
        200, clients::http::Headers{}, formats::json::ToString(response_body_value_builder.ExtractValue())};
}

}  // namespace

UTEST(Etcd, TestKeyValueStorage) {
    utest::HttpServerMock mock_server(&EtcdRequestProcessor);
    auto http_client_ptr = utest::CreateHttpClient();
    auto etcd_client_ptr = std::make_shared<etcd::impl::ClientImpl>(
        *http_client_ptr,
        etcd::ClientSettings{
            {mock_server.GetBaseUrl()},
            2,
            std::chrono::milliseconds{500},
        }
    );

    const auto empty_value = etcd_client_ptr->Get("key_with_empty_value");
    EXPECT_EQ(empty_value, std::nullopt);

    etcd_client_ptr->Put("some_key", "some_value");
    const auto some_value = etcd_client_ptr->Get("some_key");
    EXPECT_EQ(some_value, "some_value");

    etcd_client_ptr->Put("some_key", "some_new_value");
    const auto some_new_value = etcd_client_ptr->Get("some_key");
    EXPECT_EQ(some_new_value, "some_new_value");

    etcd_client_ptr->Delete("some_key");
    const auto deleted_value = etcd_client_ptr->Get("some_key");
    EXPECT_EQ(deleted_value, std::nullopt);
}

UTEST(Etcd, TestRange) {
    utest::HttpServerMock mock_server(&EtcdRequestProcessor);
    auto http_client_ptr = utest::CreateHttpClient();
    auto etcd_client_ptr = std::make_shared<etcd::impl::ClientImpl>(
        *http_client_ptr,
        etcd::ClientSettings{
            {mock_server.GetBaseUrl()},
            2,
            std::chrono::milliseconds{500},
        }
    );

    EXPECT_EQ(etcd_client_ptr->Range("some_key"), (std::vector<std::string>{}));

    etcd_client_ptr->Put("some_key_1", "some_value_1");
    etcd_client_ptr->Put("some_key_2", "some_value_2");
    etcd_client_ptr->Put("some_key_3", "some_value_3");
    const auto range_result = etcd_client_ptr->Range("some_key");
    EXPECT_EQ(range_result, (std::vector<std::string>{"some_value_1", "some_value_2", "some_value_3"}));
    etcd_client_ptr->Delete("some_key_1");
    etcd_client_ptr->Delete("some_key_2");
    etcd_client_ptr->Delete("some_key_3");
    EXPECT_EQ(etcd_client_ptr->Range("some_key"), (std::vector<std::string>{}));
}

USERVER_NAMESPACE_END
