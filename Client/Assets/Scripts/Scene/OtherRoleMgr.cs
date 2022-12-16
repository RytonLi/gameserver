using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;

public class OtherRoleMgr : MonoBehaviour
{
    public float MoveSpeed = 20f;
    public float TurnSpeed = 20f;

    private class OtherRole
    {
        //Âß¼­×´Ì¬
        public int ID;
        public Vector3 Position;
        public Vector3 Rotation;
        public int Speed;

        public GameObject Obj;
    }

    private Dictionary<int, OtherRole> m_otherRoles;

    private GameObject m_rolePrefabs;

    public void SetStatus(int roleID, Vector3 pos, Vector3 rot, int speed, bool flag = true)
    {
        OtherRole role;
        if(!m_otherRoles.TryGetValue(roleID, out role))
        {
            role = new OtherRole();
            role.ID = roleID;
            role.Obj = Instantiate<GameObject>(m_rolePrefabs);
            role.Obj.SetActive(true);
            role.Obj.transform.Find("IDField").Find("Text").GetComponent<TMP_Text>().text = role.ID.ToString();
            m_otherRoles.Add(role.ID, role);
        }

        role.Position = pos;
        role.Rotation = rot;
        role.Speed = speed;

        //Ç¿ÖÆÎ»ÒÆ
        if (flag)
        {
            role.Obj.transform.position = role.Position;
            role.Obj.transform.rotation = Quaternion.LookRotation(role.Rotation);
        }

    }

    public void RemoveRole(int roleID)
    {
        OtherRole role;
        if(m_otherRoles.TryGetValue(roleID, out role))
        {
            DestroyImmediate(role.Obj);
            m_otherRoles.Remove(roleID);
        }
    }

    public void MyUpdate()
    {
        ShowMove();
    }

    private void ShowMove()
    {
        foreach (OtherRole role in m_otherRoles.Values)
        {
            role.Obj.transform.GetComponent<Animator>().SetBool("IsWalking", role.Speed > 0 ? true : false);

            role.Obj.transform.position = Vector3.MoveTowards(role.Obj.transform.position, role.Position, MoveSpeed * Time.deltaTime);
            role.Obj.transform.rotation = Quaternion.LookRotation(Vector3.RotateTowards(role.Obj.transform.forward, role.Rotation, TurnSpeed * Time.deltaTime, 0f));
        }
    }

    public void MyFixedUpdate()
    {
        LogicMove();
    }

    private void LogicMove()
    {
        foreach(OtherRole role in m_otherRoles.Values)
        {
            if (role.Speed > 0)
            {
                float span = role.Speed * Time.deltaTime;

                role.Position.x += role.Rotation.x * span;
                role.Position.z += role.Rotation.z * span;
            }

            role.Obj.transform.Find("IDField").Find("Text").GetComponent<TMP_Text>().text = role.ID.ToString() + "\nPos: " + "(" + role.Position.x.ToString("0.00") + "," + role.Position.z.ToString("0.00") + ")";
        }
    }

    private void Start()
    {
        m_otherRoles = new Dictionary<int, OtherRole>();
        m_rolePrefabs = Resources.Load("JohnLemon", typeof(GameObject)) as GameObject;
    }
}
