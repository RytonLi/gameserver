using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CubeMgr : MonoBehaviour
{
    private class Cube
    {
        public int ID;
        public GameObject Obj;
    }

    private Dictionary<int, Cube> m_cubes;

    private GameObject m_cubePrefabs;

    public void AddCube(int id, Vector3 pos, Vector3 rot, Vector3 scale)
    {
        Cube cube = new Cube();
        cube.ID = id;
        cube.Obj = Instantiate<GameObject>(m_cubePrefabs);
        cube.Obj.SetActive(true);

        Transform trans = cube.Obj.transform;
        trans.position = pos;
        trans.rotation = Quaternion.LookRotation(rot);
        trans.localScale = scale;

        m_cubes.Add(cube.ID, cube);
    }

    public void RemoveCube(int id)
    {
        Cube cube;
        
        if(m_cubes.TryGetValue(id, out cube))
        {
            DestroyImmediate(cube.Obj);
            m_cubes.Remove(id);
        }
    }

    private void Start()
    {
        m_cubes = new Dictionary<int, Cube>();
        m_cubePrefabs = Resources.Load("Cube", typeof(GameObject)) as GameObject;
    }
}
