#include "MQTT_Manager_Paho_Async.h"

void AMQTT_Manager_Paho_Async::DeliveryCompleted(void* CallbackContext, MQTTAsync_token DeliveredToken)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	Owner->Delegate_Delivered.Broadcast(FString::FromInt(DeliveredToken));
}

int AMQTT_Manager_Paho_Async::MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTAsync_message* Message)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!Owner)
	{
		return 0;
	}

	FPahoArrived_Async StrArrived;
	StrArrived.TopicName = UTF8_TO_TCHAR(TopicName);
	StrArrived.TopicLenght = TopicLenght;
	StrArrived.Message.AppendChars(UTF8_TO_TCHAR((char*)Message->payload), Message->payloadlen);

	Owner->Delegate_Arrived.Broadcast(StrArrived);

	MQTTAsync_freeMessage(&Message);
	MQTTAsync_free(TopicName);

	return 1;
}

void AMQTT_Manager_Paho_Async::ConnectionLost(void* CallbackContext, char* Cause)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	const FString CauseString = UTF8_TO_TCHAR(Cause);
	Owner->Delegate_Lost.Broadcast(CauseString);
}

bool AMQTT_Manager_Paho_Async::SetSSLParams(FString In_Protocol, FPahoClientParams_Async In_Params)
{
	if (In_Params.Address.IsEmpty())
	{
		return false;
	}

	if (In_Protocol.IsEmpty())
	{
		return false;
	}

	if (In_Protocol == "wss" || In_Protocol == "mqtts" || In_Protocol == "ssl" || In_Protocol == "WSS" || In_Protocol == "MQTTS" || In_Protocol == "SSL")
	{
		this->SSL_Options = MQTTAsync_SSLOptions_initializer;
		this->SSL_Options.enableServerCertAuth = 0;
		this->SSL_Options.verify = 1;

		if (!In_Params.CAPath.IsEmpty() && FPaths::FileExists(In_Params.CAPath))
		{
			this->SSL_Options.CApath = TCHAR_TO_UTF8(*In_Params.CAPath);
		}

		else
		{
			this->SSL_Options.CApath = NULL;
		}

		if (!In_Params.Path_KeyStore.IsEmpty() && FPaths::FileExists(In_Params.Path_KeyStore))
		{
			this->SSL_Options.keyStore = TCHAR_TO_UTF8(*In_Params.Path_KeyStore);
		}

		else
		{
			this->SSL_Options.keyStore = NULL;
		}

		if (!In_Params.Path_TrustStore.IsEmpty() && FPaths::FileExists(In_Params.Path_TrustStore))
		{
			this->SSL_Options.trustStore = TCHAR_TO_UTF8(*In_Params.Path_TrustStore);
		}

		else
		{
			this->SSL_Options.trustStore = NULL;
		}

		if (!In_Params.Path_PrivateKey.IsEmpty() && FPaths::FileExists(In_Params.Path_PrivateKey))
		{

			this->SSL_Options.privateKey = TCHAR_TO_UTF8(*In_Params.Path_PrivateKey);
		}

		else
		{
			this->SSL_Options.privateKey = NULL;
		}

		this->SSL_Options.privateKeyPassword = In_Params.PrivateKeyPass.IsEmpty() ? NULL : TCHAR_TO_UTF8(*In_Params.PrivateKeyPass);
		this->SSL_Options.enabledCipherSuites = In_Params.CipherSuites.IsEmpty() ? NULL : TCHAR_TO_UTF8(*In_Params.CipherSuites);

		return true;
	}

	else
	{
		return false;
	}
}

void AMQTT_Manager_Paho_Async::onConnect(void* CallbackContext, MQTTAsync_successData* Response)
{

}

void AMQTT_Manager_Paho_Async::onConnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{

}

void AMQTT_Manager_Paho_Async::OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response)
{

}

void AMQTT_Manager_Paho_Async::OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{

}

void AMQTT_Manager_Paho_Async::onSend(void* CallbackContext, MQTTAsync_successData* Response)
{

}

void AMQTT_Manager_Paho_Async::onSendFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{

}

void AMQTT_Manager_Paho_Async::onConnect(void* CallbackContext, MQTTAsync_successData5* Response)
{

}

void AMQTT_Manager_Paho_Async::onConnectFailure(void* CallbackContext, MQTTAsync_failureData5* Response)
{

}

void AMQTT_Manager_Paho_Async::OnDisconnect(void* CallbackContext, MQTTAsync_successData5* Response)
{

}

void AMQTT_Manager_Paho_Async::OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData5* Response)
{

}

void AMQTT_Manager_Paho_Async::onSend(void* CallbackContext, MQTTAsync_successData5* Response)
{

}

void AMQTT_Manager_Paho_Async::onSendFailure(void* CallbackContext, MQTTAsync_failureData5* Response)
{

}