using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using TCCamp;

public class ItemButton : Button
{
    public override void OnPointerEnter(PointerEventData eventData)
    {
        m_Text.text = m_textContent;
        m_Update = true;
        Cursor.visible = false;
        Description.SetActive(true);
    }

    public override void OnPointerExit(PointerEventData eventData)
    {
        m_Update = false;
        Cursor.visible = true;
        Description.SetActive(false);
    }

    public GameObject Description;
    public Text m_Text;
    public string m_textContent;

    private bool m_Update;

    private void Update()
    {
        if (m_Update)
        {
            Description.transform.position = Input.mousePosition;
        }
    }

}
