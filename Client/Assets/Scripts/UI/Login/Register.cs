using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Google.Protobuf;
using TCCamp;
using System.Text;

public class Register : MonoBehaviour
{
    private void SendRegisterReq()
    {
        AccountCreateReq req = new AccountCreateReq();
        req.AccountID = Id.text;
        req.Password = Password.text;

        m_Client.SendMsg(((int)CLIENT_CMD.ClientCreateReq), req);
    }

    private void OnRegitserRsp(int cmd, IMessage msg)
    {
        AccountCreateRsp rsp = (AccountCreateRsp)msg;
        string str = "×¢²á·µ»Ø£º" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;
    }

    public InputField Id;
    public InputField Password;
    public InputField Name;
    public Client m_Client;
    public BoardManage m_Board;
    public GameObject m_Login;

    void Start()
    {
        GetComponent<Button>().onClick.AddListener(SendRegisterReq);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerCreateRsp), OnRegitserRsp);
    }

}
