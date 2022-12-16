using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Google.Protobuf;
using TCCamp;
using System;

public class DelayCtrl : MonoBehaviour
{
    public Client m_Client;

    public long Delay;

    private long m_timestamp;

    private bool m_flag;

    private Text m_Text;

    void Start()
    {
        m_flag = false;

        m_Text = transform.Find("Text").gameObject.GetComponent<Text>();

        OnConnect();

        //MyEventSystem.Instance.AddListener("OnConnect", OnConnect);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerPong), OnPong);
    }

    private void OnConnect()
    {
        TimerManager.Instance.AddTimer(SendPing, 1, 1);
    }

    private void SendPing()
    {
        ClientPingReq req = new ClientPingReq();
        req.Timestamp = TimeStampConvert.Now();
        m_Client.SendMsg(((int)CLIENT_CMD.ClientPing), req);
    }

    private void OnPong(int cmd, IMessage msg)
    {
        ServerPongRsp rsp = (ServerPongRsp)msg;
        Delay = (TimeStampConvert.Now() - rsp.Timestamp) / 2;
        m_Text.text = "ÑÓ³Ù£º" + Delay + "ms";
    }

}
