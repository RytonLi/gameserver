using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Google.Protobuf;
using TCCamp;
using System.Text;

public class Login : MonoBehaviour
{
    private void SendLoginReq()
    {
        AccountLoginReq req = new AccountLoginReq();
        req.AccountID = Id.text;
        req.Password = Password.text;

        m_Client.SendMsg(((int)CLIENT_CMD.ClientLoginReq), req);
    }

    private void OnLoginRsp(int cmd, IMessage msg)
    {
        AccountLoginRsp rsp = (AccountLoginRsp)msg;
        string str = "µÇÂ¼·µ»Ø£º" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        MyEventSystem.Instance.SendEvent("OnLogin");
        m_Login.SetActive(false);
    }

    public InputField Id;
    public InputField Password;
    public Client m_Client;
    public BoardManage m_Board;
    public GameObject m_Login;

    private void Start()
    {
        GetComponent<Button>().onClick.AddListener(SendLoginReq);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerLoginRsp), OnLoginRsp);
    }

}
