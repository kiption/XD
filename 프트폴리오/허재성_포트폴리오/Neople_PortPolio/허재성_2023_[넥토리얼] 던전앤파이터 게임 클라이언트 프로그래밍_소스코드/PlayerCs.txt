using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    public Animator anim;
    public CharacterController controller;
    private GameObject MyCamera;
    private GameObject FireStartPosition;
    private GameObject FireStartLook;
    public GameObject BulletObject;
    private GameObject HeadPosition;
    private float rotationSpeed = 30f;
    private float rotationSpeedY = 10f;
    public float moveSpeed = 2.5f;
    private float bulletSpeed = 20000;
    private float BulletActiveTerm = 0.0f;

    RaycastHit hit;

    private bool isJumping = false;
    private bool isSlashJumping = false;
    float WeightValue;
    void Start()
    {
        controller = GetComponent<CharacterController>();
        anim = GetComponent<Animator>();
        MyCamera = GameObject.Find("Camera");
        FireStartPosition = GameObject.Find("FirePosition");
        FireStartLook = GameObject.Find("Rifle_1");
        HeadPosition = GameObject.Find("mixamorig:Head");
        WeightValue = 0.5f;
    }

    void Update()
    {
        float mouseGetX = Input.GetAxis("Mouse X");
        float mouseGetY = Input.GetAxis("Mouse Y");
        float moveGetX = Input.GetAxis("Horizontal") * WeightValue;
        float moveGetZ = Input.GetAxis("Vertical") * WeightValue;
        transform.Rotate(Vector3.up, mouseGetX * rotationSpeed * Time.deltaTime);
        float YPosition = transform.position.y;


        anim.SetFloat("Height", transform.position.y);
        if (Physics.Raycast(transform.position, -transform.up, out hit, 2.0f))
        {

            if (hit.distance < 0.1f)
            {

                isJumping = false;
                isSlashJumping = false;
            }
        }
        if (moveGetX == 0.0f && moveGetZ == 0.0f)
        {
            anim.SetBool("Idle", true);
            WeightValue = 0.8f;
        }
        Vector3 moveDirection = new Vector3(moveGetX, 0, moveGetZ);
        moveDirection = transform.TransformDirection(moveDirection);
        controller.Move(moveDirection * Time.deltaTime * moveSpeed);

        anim.SetFloat("Movez", moveGetZ);
        anim.SetFloat("Movex", moveGetX);
        BulletActiveTerm += Time.deltaTime * 5.0f;
        PlayerAnimationByState();
        CameraUpdate();

    }

    void CameraUpdate()
    {
        Vector3 CameraOffsetStartPosition = HeadPosition.transform.position;
        Vector3 cameraOffset = new Vector3(1.4f, 0.0f, -2.5f);
        MyCamera.transform.rotation = Quaternion.LookRotation(transform.forward, transform.up)/* * Quaternion.Euler(0f, -20f, 0f);*/ ;
        Vector3 cameraPosition = CameraOffsetStartPosition + transform.up * cameraOffset.y + transform.right * cameraOffset.x + transform.forward * cameraOffset.z;
        MyCamera.transform.position = cameraPosition;
    }

    void BulletAction()
    {
        Vector3 Startposition = FireStartPosition.transform.position;
        GameObject bullet = Instantiate(BulletObject, Startposition + MyCamera.transform.forward, Quaternion.identity);
        bullet.transform.forward = MyCamera.transform.forward;
        bullet.transform.Rotate(new Vector3(1, 0, 0), 90.0f);
        Rigidbody bulletRigidbody = bullet.GetComponent<Rigidbody>();
        bulletRigidbody.AddForce(FireStartLook.transform.forward * bulletSpeed, ForceMode.Force);
        Destroy(bullet, 2.0f);
    }

    void PlayerAnimationByState()
    {
        if (Input.GetMouseButton(0) && BulletActiveTerm > 1.0f)
        {
            BulletAction();
            anim.SetBool("Shoot", true);
            BulletActiveTerm = 0.0f;
        }
        else
        {
            anim.SetBool("Shoot", false);
        }

        if (Input.GetMouseButton(1))
        {
            anim.SetBool("Slash", true);
        }
        else
        {
            anim.SetBool("Slash", false);
        }

        if (Input.GetKeyDown(KeyCode.Q))
        {
            anim.SetBool("Roll", true);
            BulletActiveTerm = -1.0f;
        }
        else
        {
            anim.SetBool("Roll", false);
        }

        if (Input.GetKeyDown(KeyCode.Space))
        {
            // Jump();
            anim.SetBool("Jump", true);
        }
        else
        {
            anim.SetBool("Jump", false);
        }

        if (Input.GetKeyDown(KeyCode.G))
        {
            moveSpeed = 1.1f;

            anim.SetBool("JumpReady", true);
        }
        if (Input.GetKeyUp(KeyCode.G))
        {
            SlashJump();
        }

        if (Input.GetMouseButton(1))
        {
            anim.SetBool("Punch", true);
        }
        else
        {
            anim.SetBool("Punch", false);
        }
      
        if (Input.GetKey(KeyCode.LeftShift))
        {
            if (WeightValue > 30.0f)
            {
                WeightValue -= 0.3f * Time.deltaTime;
            }
            else
            {
                WeightValue += 0.3f * Time.deltaTime;
            }
            anim.SetBool("RunState", true);
        }
        else
        {
           
            ;
            WeightValue = 0.8f;
            anim.SetBool("RunState", false);
        }
    }

    void SlashJump()
    {
        if (!isSlashJumping)
        {
            isSlashJumping = true;
            controller.Move(Vector3.up * 10.0f); // 점프 시 캐릭터를 위로 이동
            anim.SetBool("JumpReady", false);
        }
    }

}
