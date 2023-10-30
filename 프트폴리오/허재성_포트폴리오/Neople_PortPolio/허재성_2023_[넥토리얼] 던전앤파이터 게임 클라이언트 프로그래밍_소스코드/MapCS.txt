using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MazeMap : MonoBehaviour
{
    public int MazeWidth;   // 미로의 너비
    public int MazeHeight;  // 미로의 높이
    public float WallSize; // 벽의 두께
    public GameObject wallPrefab; // 벽 프리팹
    public GameObject PlanPrefab; // 바닥 프리팹
    public GameObject TreePrefab; // 나무 프리팹
    private GameObject RotateCubePrefab; // 나무 프리팹

    private GameObject planeHex;
    private int[,] maze; // 미로 배열

    private void Start()
    {
        maze = new int[MazeHeight, MazeWidth];
        CrateMaze();
        RotateCubePrefab = GameObject.Find("RotateCube");
    }
    void Update()
    {
        RotateCubePrefab.transform.Rotate(0,5,0);
    }
    private void CrateMaze()
    {
        // Init maze[,]
        for (int row = 0; row < MazeHeight; row++)
        {
            for (int col = 0; col < MazeWidth; col++)
            {
                maze[row, col] = 1;
            }
        }

        // 출/입구 만들어주기
        maze[0, 1] = 0;
        maze[MazeHeight - 1, MazeWidth - 2] = 0;

        GenerateMazeRecursively(1, 1);
        // Create MazeMap Plane
        for (int row = 0; row < 100; row++)
        {
            for (int col = 0; col < 100; col++)
            {
              
                Vector3 position = new Vector3(-50.0f+col *5.0f, 0, -50.0f+row * 5.0f);
                GameObject plane = Instantiate(PlanPrefab,transform);
                plane.transform.position = position;
                plane.transform.localScale = new Vector3(0.5f, 0.5f, 0.5f); // 생성된 Plane 크기 조정
            }
        }
        // Create walls 
        for (int row = 0; row < MazeHeight; row++)
        {
            for (int col = 0; col < MazeWidth; col++)
            {
                // Create Tree
                GameObject ProbObject = Instantiate(TreePrefab, new Vector3((col/2), 0, (row/2)), Quaternion.identity);
                ProbObject.transform.position = new Vector3(col * WallSize+ Random.Range(-20, 20),2, row * WallSize+ Random.Range(-20, 20));
                ProbObject.transform.Rotate(0.0f,0.0f, 0.0f);
                ProbObject.transform.localScale = new Vector3(2.0f, 2.0f,2.0f);
              
                if (maze[row, col] == 1)
                {
                    // Create wall's Objects
                    GameObject wall = Instantiate(wallPrefab, new Vector3(col * WallSize, WallSize/2, row * WallSize), Quaternion.identity);
                    wall.transform.localScale = new Vector3(WallSize, WallSize, WallSize);
                    wall.transform.Rotate(0.0f, Random.Range(0, 4) * 90.0f, 0.0f);
                    wall.transform.parent = transform;

                    // Create CheckBox - Collider
                    GameObject CheckBox = new GameObject();
                    CheckBox.AddComponent<BoxCollider>();
                    CheckBox.GetComponent<BoxCollider>().size = new Vector3(WallSize, WallSize, WallSize);
                    CheckBox.transform.position = new Vector3(col * WallSize, WallSize / 2, row * WallSize);
                }
            }
        }
    }
    // Array Shuffle
    private void ArrayShuffle(int[] array)
    {
        for (int i = array.Length - 1; i > 0; i--)
        {
            int rd = Random.Range(0, i + 1);
            int temp = array[i];
            array[i] = array[rd];
            array[rd] = temp;
        }
    }

    // 미로를 만들기 위한 재귀 알고리즘
    private void GenerateMazeRecursively(int row, int col)
    {
        // 방향 배열 ->      Up    Down    Left    Right
        int[] directions = { 0,     1,      2,      3 };
        // 방향 Shuffle
        ArrayShuffle(directions);

        foreach (int dir in directions)
        {
            int newRow = row;
            int newCol = col;

            // 셀을 이동
            switch (dir)
            {
                case 0: 
                    newRow-=2;  // Up
                    break;
                case 1: 
                    newRow+=2;  // Down
                    break;
                case 2: 
                    newCol-=2;  // Left
                    break;
                case 3: 
                    newCol+=2;  // Right
                    break;
            }

            // 새로운 셀이 미로 범위안에 있는지 확인
            if (newRow < 0 || newRow >= MazeHeight || newCol < 0 || newCol >= MazeWidth)
            {
                continue;
            }

            // 새로운 셀이 이미 방문된 곳인지 확인.
            if (maze[newRow, newCol] == 0)
            {
                continue;
            }

            // 현재 셀과 새로운 셀 사이의 공간을 만든다.
            if (dir == 0)
            { 
                maze[row - 1, col] = 0;// Up
            }
            else if (dir == 1)
            { 
                maze[row + 1, col] = 0;// Down
            }
            else if (dir == 2)
            { 
                maze[row, col - 1] = 0;// Left
            }
            else if (dir == 3)
            { 
                maze[row, col + 1] = 0;// Right
            }
       
            maze[newRow, newCol] = 0;

            // 새로운 셀을 현재의 셀로 배치하여 재귀호출.
            GenerateMazeRecursively(newRow, newCol);
        }
    }
}