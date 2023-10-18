using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MineEnermy : MonoBehaviour
{
    private GameObject PlayerObject;
    private float jumpTimer = 0.0f;
    private float jumpInterval = 2f;

    private GameObject ExplosObjects;
    public GameObject ExplosionPreb;
    private float ParticleForce = 5;
    private float LifeTime = 5.0f;
    public Animator anim;
    // Start is called before the first frame update
    void Start()
    {
        PlayerObject = GameObject.Find("Ch44_nonPBR 1");
        anim = GetComponent<Animator>();
        Vector3 Position = transform.position;
        Position.y = 0.0f;
    }

    // Update is called once per frame
    void Update()
    {
        Vector3 playerPosition = PlayerObject.transform.position;

        if ( (playerPosition.z - transform.position.z) < 10.0f)
        {
            Vector3 moveDirection = playerPosition - transform.position;
            moveDirection.Normalize();
          //  transform.position += moveDirection * 1.5f * Time.deltaTime;
           // transform.position = new Vector3(transform.position.x, transform.position.y, transform.position.z);
            //anim.SetBool("Move",true);
        }
       
        Jump();
    }



    void Jump()
    {

        jumpTimer += Time.deltaTime;
        
        if (jumpTimer >= jumpInterval)
        {
            Rigidbody Minrigidb = GetComponent<Rigidbody>();
            anim.SetBool("Jump", true);
            Minrigidb.AddForce(Vector3.up * 6f, ForceMode.Impulse);
            jumpTimer = 0.0f;
        }
        else
        {
            anim.SetBool("Jump", false);
        }

    }
    
    void ExplosionAction()
    {

        for (int i = 0; i < 80; i++)
        {
            Vector3 StartPos = transform.position;
            ExplosObjects = Instantiate(ExplosionPreb, new Vector3(StartPos.x, StartPos.y, StartPos.z), Quaternion.identity);
            ExplosObjects.transform.Rotate(10, 10, 10);
            Rigidbody Exrigidby = ExplosObjects.GetComponent<Rigidbody>();

            Vector3 RandomDir = Random.insideUnitSphere * ParticleForce;
            Exrigidby.AddForce(RandomDir * 10, ForceMode.Impulse);
            // Lifetime 초 뒤에 사라짐
            Destroy(ExplosObjects, LifeTime);
        }

    }

}
