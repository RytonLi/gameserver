using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TimerCtrl : MonoBehaviour
{

    void Start()
    {
        
    }

    void Update()
    {
        TimerManager.Instance.Update();
    }
}
