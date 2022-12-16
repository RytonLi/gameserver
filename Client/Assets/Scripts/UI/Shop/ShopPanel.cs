using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf;
using TCCamp;
using UnityEngine.Events;
using UnityEngine.UI;
using System;

public class ShopPanel : MonoBehaviour
{

    private int SelectedIndex;
    private int BuyNum;
    private class ShopItemObj{
        public PBShopItem item;
        public GameObject obj;
    }

    private SortedDictionary<int, ShopItemObj> m_Items = new SortedDictionary<int, ShopItemObj>();

    //处理购买响应
    private void OnBuyRsp(int cmd, IMessage msg)
    {
        RoleBuyItemRsp rsp = (RoleBuyItemRsp)msg;
        string str = "购买返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        SendUpdateShopListReq();
        SendMoneyReq();
    }

    //确认购买，发送购买请求
    private void ClickSummitBuyButton()
    {
        RoleBuyItemReq req = new RoleBuyItemReq();
        req.ItemID = SelectedIndex;
        req.ItemCount = BuyNum;

        m_Client.SendMsg(((int)CLIENT_CMD.ClientRoleBuyItemReq), req);
        MsgBox.SetActive(false);
    }

    private void ClickCancelBuyButton()
    {
        MsgBox.SetActive(false);
    }

    private void ClickSummitNumButton()
    {
        bool result = int.TryParse(m_NumInput.text, out BuyNum);
        if(result == true)
        {
            PBShopItem selectedItem = m_Items[SelectedIndex].item;
            m_MsgText.text = "您确定要花费" + selectedItem.Price * BuyNum +
                "购买 " + BuyNum + "*" + selectedItem.Id.ToString() + " 吗？";
            MsgBox.SetActive(true);
            BuyNumBox.SetActive(false);
        }
        else
        {
            string str = "请输入数字...";
            m_Board.AddMessage(str);
        }
    }

    private void ClickCancelNumButton()
    {
        BuyNumBox.SetActive(false);
    }

    private void ClickBuyButton()
    {
        if (SelectedIndex == -1) return;

        BuyNumBox.SetActive(true);
    }

    //发送更新金钱请求
    private void SendMoneyReq()
    {
        m_Client.SendMsg(((int)CLIENT_CMD.ClientUpdateMoneyReq), null);
    }

    //处理更新金钱响应
    private void OnMoneyRsp(int cmd, IMessage msg)
    {
        UpdateMoneyRsp rsp = (UpdateMoneyRsp)msg;
        string str = "更新金钱返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        MoneyText.text = rsp.Money.ToString();

    }

    private void SendUpdateShopListReq()
    {
        m_Client.SendMsg(((int)CLIENT_CMD.ClientUpdateShoplistReq), null);
    }

    //收到商店列表响应回调，缓存商店列表
    private void OnUpdateShopListRsp(int cmd, IMessage msg)
    {
        UpdateShopListRsp rsp = (UpdateShopListRsp)msg;
        string str = "更新商店列表返回：" + rsp.Result.ToString();
        m_Board.AddMessage(str);

        if (rsp.Result != 0) return;

        foreach (ShopItemObj cur in m_Items.Values)
        {
            Destroy(cur.obj);
        }
        m_Items.Clear();

        foreach(PBShopItem item in rsp.Items)
        {
            ShopItemObj addItem = new ShopItemObj();
            addItem.item = item;
            m_Items.Add(item.Id, addItem);
        }

        CreateShopItemObj();
    }

    private void OutlineSelectItem()
    {
        Outline outline = m_Items[SelectedIndex].obj.GetComponent<Outline>();
        outline.effectDistance = new Vector2(3f, 3f);
        foreach(int i in m_Items.Keys)
        {
            if (i == SelectedIndex) continue;

            outline = m_Items[i].obj.GetComponent<Outline>();
            outline.effectDistance = new Vector2(0f, 0f);
        }
    }

    private void ResetShopSelectItem()
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

    //根据商店列表生成商店物品对象
    private void CreateShopItemObj()
    {
        foreach(ShopItemObj cur in m_Items.Values)
        {
            cur.obj = Instantiate<GameObject>(m_ItemPrefabs, Content.transform);
            cur.obj.SetActive(true);

            //设置物品图标
            GameObject itemName = cur.obj.transform.Find("ItemName").gameObject;
            GameObject itemNum = cur.obj.transform.Find("ItemNum").gameObject;
            itemName.GetComponent<Text>().text = cur.item.Id.ToString();
            itemNum.GetComponent<Text>().text = "";

            //设置物品按钮
            int tmp = cur.item.Id;
            ItemButton itemButton = cur.obj.GetComponent<ItemButton>();
            itemButton.Description = Description;
            itemButton.m_Text = Description.GetComponentInChildren<Text>();
            itemButton.m_textContent = "<color=red>物品ID：</color>" + cur.item.Id
                + "\n<color=red>价格：</color>" + cur.item.Price;
            if(cur.item.GlobalLimitMaxCount == -1)
            {
                itemButton.m_textContent += "\n<color=red>全服限购：</color>" + "不限量";
            }
            else
            {
                itemButton.m_textContent += "\n<color=red>全服限购：</color>" + cur.item.GlobalLimitCount + "/" + cur.item.GlobalLimitMaxCount;
            }

            if (cur.item.PersonalLimitMaxCount == -1)
            {
                itemButton.m_textContent += "\n<color=red>个人限购：</color>" + "不限量";
            }
            else
            {
                itemButton.m_textContent += "\n<color=red>个人限购：</color>" + cur.item.PersonalLimitCount + "/" + cur.item.PersonalLimitMaxCount;
            }

            if(cur.item.StartTime == -1)
            {
                itemButton.m_textContent += "\n<color=red>开始时间：</color>" + "不限";
            }
            else
            {
                DateTime dt = TimeStampConvert.GetDateTime(cur.item.StartTime);
                itemButton.m_textContent += "\n<color=red>开始时间：</color>" + dt.ToShortDateString();
            }

            if (cur.item.EndTime == -1)
            {
                itemButton.m_textContent += "\n<color=red>开始时间：</color>" + "不限";
            }
            else
            {
                DateTime dt = TimeStampConvert.GetDateTime(cur.item.EndTime);
                itemButton.m_textContent += "\n<color=red>结束时间：</color>" + dt.ToShortDateString();
            }

            itemButton.onClick.AddListener(() => { SetSelectedIndex(tmp); });
        }
    }

    public Client m_Client;
    public BoardManage m_Board;
    public GameObject Content;
    public GameObject Description;
    public Text MoneyText;
    public Button BuyButton;
    public GameObject MsgBox;
    public GameObject BuyNumBox;

    private GameObject m_ItemPrefabs;

    private Text m_MsgText;
    private Button m_SummitBuyButton;
    private Button m_CancelBuyButton;

    private InputField m_NumInput;
    private Button m_SummitNumButton;
    private Button m_CancelNumButton;

    private void Start()
    {
        MyEventSystem.Instance.AddListener("SendUpdateShopListReq", SendUpdateShopListReq);
        MyEventSystem.Instance.AddListener("ResetShopSelectItem", ResetShopSelectItem);
        MyEventSystem.Instance.AddListener("SendMoneyReq", SendMoneyReq);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerUpdateShoplistRsp), OnUpdateShopListRsp);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerRoleBuyItemRsp), OnBuyRsp);
        EventModule.Instance.AddNetEvent(((int)SERVER_CMD.ServerUpdateMoneyRsp), OnMoneyRsp);

        m_ItemPrefabs = Resources.Load("GameItem", typeof(GameObject)) as GameObject;

        m_MsgText = MsgBox.transform.Find("MsgText").gameObject.GetComponent<Text>();
        m_SummitBuyButton = MsgBox.transform.Find("Summit").gameObject.GetComponent<Button>();
        m_CancelBuyButton = MsgBox.transform.Find("Cancel").gameObject.GetComponent<Button>();

        m_NumInput = BuyNumBox.transform.Find("NumInput").gameObject.GetComponent<InputField>();
        m_SummitNumButton = BuyNumBox.transform.Find("Summit").gameObject.GetComponent<Button>();
        m_CancelNumButton = BuyNumBox.transform.Find("Cancel").gameObject.GetComponent<Button>();

        BuyButton.onClick.AddListener(ClickBuyButton);

        m_SummitBuyButton.onClick.AddListener(ClickSummitBuyButton);
        m_CancelBuyButton.onClick.AddListener(ClickCancelBuyButton);

        m_SummitNumButton.onClick.AddListener(ClickSummitNumButton);
        m_CancelNumButton.onClick.AddListener(ClickCancelNumButton);

        SelectedIndex = -1;
    }
}
