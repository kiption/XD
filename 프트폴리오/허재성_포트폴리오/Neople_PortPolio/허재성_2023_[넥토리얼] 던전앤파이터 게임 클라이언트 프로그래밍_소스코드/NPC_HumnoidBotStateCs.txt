using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Goliathstate : MonoBehaviour
{
    public GameObject PlayerObject;
    public GameObject BulletObject;
    private float bulletSpeed = 1000;
    private GameObject LazerPosion;
    public Animator anim;
    private float followSpeed = 200.0f;
    private float moveSpeed;
    private Vector3 randomMovingScalra; // 랜덤 목적지
    private float randomMoveInterval = 3.0f; // 랜덤 이동 간격
    private float randomMoveTimer = 0.0f; // 랜덤 이동 타이머
    public CharacterController controller;
    //RayCast
    RaycastHit hit;

    void Start()
    {
        PlayerObject = GameObject.Find("Ch44_nonPBR 1");
        LazerPosion = GameObject.Find("mixamorig:HeadTop_End");
        controller = GetComponent<CharacterController>();
        anim = GetComponent<Animator>();
    }

    void Update()
    {
        bool isColliding = false;

        Vector3 HumanMonsterPos = transform.position;
        
        Vector3 PlayerPos = PlayerObject.transform.position;
        HumanMonsterPos.y += 1.65f;
        // 주변을 서성거리다가 랜덤하게 이동
        // 랜덤하게 이동하다가 레이에 내가 충돌하면 날 바라보고 발사
        if (Physics.Raycast(HumanMonsterPos, transform.forward, out hit, 5.0f))
        {
            if (hit.collider.CompareTag("Player"))
            {
                anim.SetBool("Looking", true);
                BulletAction(HumanMonsterPos, transform.forward);
            }

            transform.LookAt(PlayerObject.transform, Vector3.up);
            moveSpeed = 0.0f;
        }
        else
        {
            // 랜덤 이동 로직
            randomMoveTimer += Time.deltaTime;
            if (randomMoveTimer >= randomMoveInterval)
            {
                SetRandomDestination();
                randomMoveTimer = 0.0f;
                moveSpeed = 0.0f;
                anim.SetBool("Test", false);
            }

            Vector3 moveDirection = randomMovingScalra - transform.position;
            moveDirection.Normalize();

            moveSpeed = Time.deltaTime * 40.5f;
            moveDirection = transform.TransformDirection(moveDirection);
            moveDirection *= moveSpeed;
            controller.Move(moveDirection * Time.deltaTime);
            anim.SetBool("Looking", false);


        }
        

        anim.SetFloat("MoveSpeed", moveSpeed);

    }
    void SetRandomDestination()
    {
        float randomX = Random.Range(1f, 250f);
        float randomZ = Random.Range(-200f, 200f);
        randomMovingScalra = new Vector3(randomX, 0f, randomZ);
    }
    void BulletAction(Vector3 spawnPosition, Vector3 direction)
    {
        GameObject bullet = Instantiate(BulletObject, new Vector3(spawnPosition.x, spawnPosition.y, spawnPosition.z), Quaternion.identity);
        bullet.transform.forward = direction;
        bullet.transform.Rotate(new Vector3(1, 0, 0), 90.0f);
        Rigidbody bulletRigidbody = bullet.GetComponent<Rigidbody>();
        bulletRigidbody.AddForce(direction * bulletSpeed, ForceMode.Force);
        Destroy(bullet, 1.2f);
    }
}
