// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MQTT_Async_Includes.h"

#include "MQTT_Manager_Paho_Async.generated.h"

UCLASS()
class FF_MQTT_ASYNC_API AMQTT_Manager_Paho_Async : public AActor
{
	GENERATED_BODY()
	
private:	

	MQTTAsync Client = nullptr;
	MQTTAsync_connectOptions Connection_Options;
	MQTTAsync_SSLOptions SSL_Options;
	FPahoClientParams_Async Client_Params;

#pragma region CALLBACKS

	virtual bool SetSSLParams(FString In_Protocol, FPahoClientParams_Async In_Params);

	static void onConnect(void* CallbackContext, MQTTAsync_successData* Response);
	static void onConnectFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	static void OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	static void onSend(void* CallbackContext, MQTTAsync_successData* Response);
	static void onSendFailure(void* CallbackContext, MQTTAsync_failureData* Response);

	static void onConnect(void* CallbackContext, MQTTAsync_successData5* Response);
	static void onConnectFailure(void* CallbackContext, MQTTAsync_failureData5* Response);
	static void OnDisconnect(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData5* Response);
	static void onSend(void* CallbackContext, MQTTAsync_successData5* Response);
	static void onSendFailure(void* CallbackContext, MQTTAsync_failureData5* Response);

	static void DeliveryCompleted(void* CallbackContext, MQTTAsync_token DeliveredToken);
	static int MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTAsync_message* Message);
	static void ConnectionLost(void* CallbackContext, char* Cause);
	
#pragma endregion CALLBACKS

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	AMQTT_Manager_Paho_Async();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Delivered_Async Delegate_Delivered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Arrived_Async Delegate_Arrived;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Lost_Async Delegate_Lost;

	UFUNCTION(BlueprintPure)
	virtual FPahoClientParams_Async GetClientParams();

	UFUNCTION(BlueprintCallable)
	virtual void MQTT_Async_Destroy();

	UFUNCTION(BlueprintCallable)
	virtual void MQTT_Async_Init(FDelegate_Paho_Connection_Async DelegateConnection, FPahoClientParams_Async In_Params);

	UFUNCTION(BlueprintCallable)
	virtual bool MQTT_Async_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS_Async In_QoS = EMQTTQOS_Async::QoS_0, int32 In_Retained = 0);

	UFUNCTION(BlueprintCallable)
	virtual bool MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS_Async In_QoS = EMQTTQOS_Async::QoS_0);

};
