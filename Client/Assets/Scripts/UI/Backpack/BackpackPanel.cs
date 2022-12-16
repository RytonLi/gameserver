using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Google.Protobuf;
using TCCamp;

public class BackpackPanel : MonoBehaviour
{
    private int SelectedIndex;

    private class BagItemObj
    {
        public PBBagItem item;
        public GameObject obj;
    }

    private SortedDictionary<int, BagItemObj> m_Items = new SortedDictionary<int, BagItemObj>();

    //发送更新背包请求
    private void SendUpdateBagListReq()
    {
        m_Client.SendMsg(((int)CLIENT_CMD.ClientUpdateBaglistReq), null);
    }

    //处理更新背包响应
    private void OnUpdateBagListRsp(int cmd, IMessage msg)
    {
        UpdateBagListRsp rsp = (UpdateBagListRsp)msg;
        string str = "更新背包列表返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        foreach (BagItemObj cur in m_Items.Values)
        {
            Destroy(cur.obj);
        }
        m_Items.Clear();

        foreach (PBBagItem item in rsp.Items)
        {
            BagItemObj addItem = new BagItemObj();
            addItem.item = item;
            m_Items.Add(item.Id, addItem);
        }
        CreateBackpackItemObj();
    }

    private void CreateBackpackItemObj()
    {
        foreach(BagItemObj cur in m_Items.Values)
        {
            cur.obj = Instantiate<GameObject>(m_ItemPrefabs, Content.transform);
            cur.obj.SetActive(true);

            //设置物品图标
            GameObject itemName = cur.obj.transform.Find("ItemName").gameObject;
            GameObject itemNum = cur.obj.transform.Find("ItemNum").gameObject;
            itemName.GetComponent<Text>().text = cur.item.Id.ToString();
            itemNum.GetComponent<Text>().text = cur.item.Num.ToString();

            //设置物品按钮
            int tmp = cur.item.Id;
            ItemButton itemButton = cur.obj.GetComponent<ItemButton>();
            itemButton.Description = Description;
            itemButton.m_Text = Description.GetComponentInChildren<Text>();
            itemButton.m_textContent = "<color=red>物品ID：</color>" + cur.item.Id;
            itemButton.onClick.AddListener(() => { SetSelectedIndex(tmp); });
        }
    }

    private void OutlineSelectItem()
    {
        Outline outline = m_Items[SelectedIndex].obj.GetComponent<Outline>();
        outline.effectDistance = new Vector2(3f, 3f);
        foreach (int i in m_Items.Keys)
        {
            if (i == SelectedIndex) continue;

            outline = m_Items[i].obj.GetComponent<Outline>();
            outline.effectDistance = new Vector2(0f, 0f);
        }
    }

    private void ResetBackpackSelectItem()
    {
        SelectedIndex = -1;
        foreach (int i in m_Items.Keys)
        {
            Outline outline = m_Items[i].obj.GetComponent<Outline>();
            outline.effectDistance = new Vector2(0f, 0f);
        }
    }

    private void SetSelectedIndex(int index)
    {
        SelectedIndex = index;
        OutlineSelectItem();
    }

    public Client m_Client;
    public BoardManage m_Board;
    public GameObject Content;
    public GameObject Description;
    public ShopPanel m_Shop;

    private GameObject m_ItemPrefabs;


    void Start()
    {
        MyEventSystem.Instance.AddListener("SendUpdateBagListReq", SendUpdateBagListReq);
        MyEventSystem.Instance.AddListener("ResetBackpackSelectItem", ResetBackpackSelectItem);

        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerUpdateBaglistRsp), OnUpdateBagListRsp);

        m_ItemPrefabs = Resources.Load("GameItem", typeof(GameObject)) as GameObject;
    }
}
