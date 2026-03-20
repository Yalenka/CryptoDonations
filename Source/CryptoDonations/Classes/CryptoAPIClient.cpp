#include "CryptoAPIClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

// ===============================
// CREATE PAYMENT
// ===============================
void UCryptoAPIClient::CreatePayment(float Amount, FString Currency, FString UserId)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + "/create-payment");
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    FString Body = FString::Printf(
        TEXT("{\"amount\": %.2f, \"currency\": \"%s\", \"userId\": \"%s\"}"),
        Amount,
        *Currency,
        *UserId
    );

    Request->SetContentAsString(Body);

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoAPIClient::OnCreatePaymentResponse);
    Request->ProcessRequest();
}

// ===============================
// CREATE PAYMENT RESPONSE
// ===============================
void UCryptoAPIClient::OnCreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreatePayment failed"));
        return;
    }

    FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("CreatePayment Response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FDonationSession Session;

        // Safe parsing
        if (JsonObject->HasField("payment_id"))
            Session.PaymentID = JsonObject->GetStringField("payment_id");

        if (JsonObject->HasField("pay_address"))
            Session.PayAddress = JsonObject->GetStringField("pay_address");

        if (JsonObject->HasField("pay_amount"))
            Session.PayAmount = JsonObject->GetStringField("pay_amount");

        if (JsonObject->HasField("qr_code_url"))
            Session.QRCodeURL = JsonObject->GetStringField("qr_code_url");

        Session.Status = "waiting";

        OnPaymentCreated.Broadcast(Session);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse CreatePayment JSON"));
    }
}

// ===============================
// CHECK PAYMENT STATUS
// ===============================
void UCryptoAPIClient::CheckPaymentStatus(FString PaymentID)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + "/payment-status/" + PaymentID);
    Request->SetVerb("GET");

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoAPIClient::OnStatusResponse);
    Request->ProcessRequest();
}

// ===============================
// STATUS RESPONSE
// ===============================
void UCryptoAPIClient::OnStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Status request failed"));
        return;
    }

    FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("Status Response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FString Status = "unknown";
        int32 Reward = 0;

        if (JsonObject->HasField("status"))
            Status = JsonObject->GetStringField("status");

        if (JsonObject->HasField("reward"))
            Reward = JsonObject->GetIntegerField("reward");

        OnPaymentStatus.Broadcast(Status, Reward);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse Status JSON"));
    }
}
