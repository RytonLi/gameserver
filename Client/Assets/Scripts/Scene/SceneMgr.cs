using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf;
using TCCamp;

public class SceneMgr : MonoBehaviour
{
    public Client m_Client;
    public BoardManage m_Board;
    public MyRole m_myRole;
    public OtherRoleMgr m_otherRoleMgr;
    public CubeMgr m_cubeMgr;

    private bool m_isEnable;
    private Vector2 m_preInput;

    void Start()
    {
        Time.fixedDeltaTime = 1 / 30f;

        m_isEnable = false;
        m_preInput = new Vector2(0f, 0f);

        MyEventSystem.Instance.AddListener("OnLogin", OnLogin);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerGetRolelistRsp), OnGetRoleListRsp);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerCreateRoleRsp), OnCreateRoleRsp);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerGetRoledataRsp), OnSelectRoleRsp);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerEnterSceneRsp), OnEnterSceneRsp);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerRoleMoveSyncAnnounce), OnSyncAnnounce);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerRoleExitSceneAnnounce), OnRoleExitAnnounce);
    }

    void Update()
    {
        if (m_isEnable)
        {
            m_myRole.MyUpdate();
            m_otherRoleMgr.MyUpdate();
        }
    }

    private void FixedUpdate()
    {
        if (m_isEnable)
        {
            m_myRole.MyFixedUpdate();
            m_otherRoleMgr.MyFixedUpdate();
        }
    }

    private void OnLogin()
    {
        SendGetRoleListReq();
        //SendSelectRoleReq(0);
    }

    private void SendGetRoleListReq()
    {
        m_Client.SendMsg(((int)CLIENT_CMD.ClientGetRolelistReq), null);
    }

    private void OnGetRoleListRsp(int cmd, IMessage msg)
    {
        GetRoleListRsp rsp = (GetRoleListRsp)msg;
        string str = "获取角色列表返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        if(rsp.RoleIDs.Count == 0)
        {
            SendCreateRoleReq();
        }
        else
        {
            SendSelectRoleReq(rsp.RoleIDs[0]);
        }
    }

    private void SendCreateRoleReq()
    {
        m_Client.SendMsg(((int)CLIENT_CMD.ClientCreateRoleReq), null);
    }

    private void OnCreateRoleRsp(int cmd, IMessage msg)
    {
        RoleCreateRsp rsp = (RoleCreateRsp)msg;
        string str = "创建角色返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        SendGetRoleListReq();
    }

    private void SendSelectRoleReq(int roleID)
    {
        GetRoleDataReq req = new GetRoleDataReq();
        req.RoleID = roleID;
        m_Client.SendMsg(((int)CLIENT_CMD.ClientGetRoledataReq), req);
    }

    private void OnSelectRoleRsp(int cmd, IMessage msg)
    {
        GetRoleDataRsp rsp = (GetRoleDataRsp)msg;
        string str = "选择角色返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        m_myRole.SetID(rsp.Data.RoleID);

        SendEnterSceneReq();
    }

    private void SendEnterSceneReq()
    {
        EnterSceneReq req = new EnterSceneReq();
        req.SceneID = 0;
        m_Client.SendMsg(((int)CLIENT_CMD.ClientEnterSceneReq), req);
    }

    private void OnEnterSceneRsp(int cmd, IMessage msg)
    {
        EnterSceneRsp rsp = (EnterSceneRsp)msg;
        string str = "进入场景返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        m_isEnable = true;
    }

    private void OnSyncAnnounce(int cmd, IMessage msg)
    {
        SceneSyncAnnounce ann = (SceneSyncAnnounce)msg;

        foreach (PBRoleMoveSyncData data in ann.Datas)
        {
            if (data.RoleID == m_myRole.GetID())
            {
                m_myRole.SetStatus(
                    new Vector3((float)data.Status.Pos.X, 0f, (float)data.Status.Pos.Y),
                    new Vector3((float)data.Status.Rot.X, 0f, (float)data.Status.Rot.Y),
                    data.Status.Speed,
                    true
                    );
            }
            else
            {
                m_otherRoleMgr.SetStatus(
                    (int)data.RoleID,
                    new Vector3((float)data.Status.Pos.X, 0f, (float)data.Status.Pos.Y),
                    new Vector3((float)data.Status.Rot.X, 0f, (float)data.Status.Rot.Y),
                    data.Status.Speed,
                    true
                    );
            }
        }

        foreach (CubeTransformInfo info in ann.CubeInfos)
        {
            m_cubeMgr.AddCube(
                info.ID,
                new Vector3((float)info.Pos.X, 0.5f, (float)info.Pos.Y),
                new Vector3((float)info.Rot.X, 0f, (float)info.Rot.Y),
                new Vector3((float)info.Scale.X, 1f, (float)info.Scale.Y)
                );
        }
    }

    private void OnRoleExitAnnounce(int cmd, IMessage msg)
    {
        ExitSceneAnnounce ann = (ExitSceneAnnounce)msg;

        foreach (int roleID in ann.RoleIds)
        {
            m_otherRoleMgr.RemoveRole(roleID);
        }

        foreach(int cubeID in ann.CubeIds)
        {
            m_cubeMgr.RemoveCube(cubeID);
        }
    }
}
