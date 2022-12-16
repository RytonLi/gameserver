using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;
using Google.Protobuf;
using TCCamp;

public class MyRole : MonoBehaviour
{
    public Client m_Client;

    public float MoveSpeed = 20f;
    public float TurnSpeed = 20f;
    public int RoleMoveSpeed = 2;

    private Transform m_IDField;

    private int m_id;

    //当前逻辑状态
    private Vector3 m_position;
    private Vector3 m_rotation;
    private int m_speed;

    private Vector2 m_preInput;

    public void SetID(int id)
    {
        m_id = id;
        m_IDField.Find("Text").GetComponent<TMP_Text>().text = m_id.ToString();
    }

    public int GetID()
    {
        return m_id;
    }

    public void SetStatus(Vector3 pos, Vector3 rot, int speed, bool flag = false)
    {
        m_position = pos;
        m_rotation = rot;
        m_speed = speed;

        //强制位移
        if (flag)
        {
            transform.position = m_position;
            transform.rotation = Quaternion.LookRotation(m_rotation);
        }
    }

    public void MyUpdate()
    {
        ShowMove();
    }

    private void ShowMove()
    {
        transform.GetComponent<Animator>().SetBool("IsWalking", m_speed > 0 ? true : false);

        transform.position = Vector3.MoveTowards(transform.position, m_position, MoveSpeed * Time.deltaTime);
        transform.rotation = Quaternion.LookRotation(Vector3.RotateTowards(transform.forward, m_rotation, TurnSpeed * Time.deltaTime, 0f));
    }

    public void MyFixedUpdate()
    {
        HandleInput();
        LogicMove();

        m_IDField.Find("Text").GetComponent<TMP_Text>().text = m_id.ToString() + "\nPos: " + "(" + m_position.x.ToString("0.00") + "," + m_position.z.ToString("0.00") + ")";
    }

    private void HandleInput()
    {
        //System.Random rd = new System.Random();
        //float horizontal = (float)rd.NextDouble() * 2 - 1;
        //float vertical = (float)rd.NextDouble() * 2 - 1;

        float horizontal = Input.GetAxis("Horizontal");
        float vertical = Input.GetAxis("Vertical");

        Vector2 input = new Vector2(horizontal, vertical);
        input.Normalize();

        bool enter = !Mathf.Approximately(horizontal, 0f) || !Mathf.Approximately(vertical, 0f);

        if (m_preInput != input)
        {
            SendInput(input, enter);
            m_preInput = input;
        }
    }

    private void SendInput(Vector2 input, bool enter)
    {
        //Debug.Log("Send input" + input.ToString());
        RoleMoveSyncReq req = new RoleMoveSyncReq();
        req.Status = new PBRoleMoveSyncData();
        req.Status.Status = new PBRoleStatus();
        req.Status.Status.Pos = new PBPosition();
        req.Status.Status.Rot = new PBRotation();
        req.Status.RoleID = m_id;
        req.Status.Status.Pos.X = m_position.x;
        req.Status.Status.Pos.Y = m_position.z;

        if (enter)
        {
            req.Status.Status.Rot.X = input.x;
            req.Status.Status.Rot.Y = input.y;
        }
        else
        {
            req.Status.Status.Rot.X = m_rotation.x;
            req.Status.Status.Rot.Y = m_rotation.z;
        }

        req.Status.Status.Speed = enter ? RoleMoveSpeed : 0;

        m_Client.SendMsg(((int)CLIENT_CMD.ClientRoleMoveSyncReq), req);
    }

    private void OnInputRsp(int cmd, IMessage msg)
    {
        RoleMoveSyncRsp rsp = (RoleMoveSyncRsp)msg;

        Debug.Log("on input rsp");
    }

    private void LogicMove()
    {
        if (m_speed > 0)
        {
            float span = m_speed * Time.deltaTime;

            m_position.x += m_rotation.x * span;
            m_position.z += m_rotation.z * span;
        }
    }

    private void Start()
    {
        m_IDField = transform.Find("IDField");
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerRoleMoveSyncRsp), OnInputRsp);
    }
}
