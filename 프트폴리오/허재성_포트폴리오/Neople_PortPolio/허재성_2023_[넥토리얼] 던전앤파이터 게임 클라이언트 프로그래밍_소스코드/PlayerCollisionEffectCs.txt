using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CollisionStatePlayer : MonoBehaviour
{
    // Start is called before the first frame update
    RaycastHit hit;
    private GameObject MyCamera;
    private GameObject FireStartPosition;
    public GameObject BulletObject;
    public GameObject ExplosionPreb;
    public GameObject longExplosionPreb;
    private GameObject HeadPosition;
    private float ParticleForce = 5;
    private float DieParticleForce = 10;

    private float LifeTime = 0.5f;
    private float longLifeTime = 0.9f;
    float raycastDistance = 100.0f;
    int HumanBotHP = 100;

    int MineBotHP = 200;
    int FlyBotHP = 20;
   
    void Start()
    {
        FireStartPosition = GameObject.Find("Rifle_1");
        MyCamera = GameObject.Find("Camera");
        HeadPosition = GameObject.Find("mixamorig:Head");
    }

    // Update is called once per frame
    void Update()
    {
        Attacked();
        ObjectDie();
        
    }

    void Attacked()
    {
        bool isColliding = false;
        bool isHUmanColliding = false;
        bool isFlyColliding = false;
        Vector3 Startposition = FireStartPosition.transform.position;
        if (Physics.Raycast(Startposition, FireStartPosition.transform.forward, out hit, raycastDistance))
        {

            if (hit.collider.CompareTag("MineBot"))
            {
                isColliding = true;

            }
            if (hit.collider.CompareTag("HumanBot"))
            {
                isHUmanColliding = true;

            }
            if (hit.collider.CompareTag("FlyBot"))
            {
                isFlyColliding = true;

            }
        }
        if (isColliding == true)
        {
            if (Input.GetMouseButton(0))
            {
                MineBotHP -= 15;
                Vector3 CollisionPosition = hit.collider.gameObject.transform.position;
                UnderattackExplosionAction(new Vector3(CollisionPosition.x, CollisionPosition.y + 0.3f, CollisionPosition.z));
               
            }
        }
        if (isHUmanColliding == true)
        {
            if (Input.GetMouseButton(0))
            {
                HumanBotHP -= 10;
                Vector3 CollisionPosition = hit.collider.gameObject.transform.position;
                UnderattackExplosionAction(new Vector3(CollisionPosition.x, CollisionPosition.y + 1.3f, CollisionPosition.z));

            }
        }
        if (isFlyColliding == true)
        {
            if (Input.GetMouseButton(1))
            {
                FlyBotHP -= 20;
                Vector3 CollisionPosition = hit.collider.gameObject.transform.position;
                UnderattackExplosionAction(new Vector3(CollisionPosition.x, CollisionPosition.y, CollisionPosition.z));
                
            }
        }
    }
    void ObjectDie()
    {
        if (MineBotHP <= 0)
        {
            Debug.Log("Die!");
            DieExplosionAction(hit.collider.gameObject.transform.position);
            Destroy(hit.collider.gameObject);
            MineBotHP = 200;
        }
        if (HumanBotHP <= 0)
        {
            Debug.Log("HDie!");
            DieExplosionAction(hit.collider.gameObject.transform.position);
            Destroy(hit.collider.gameObject);
           
            HumanBotHP = 100;
        }
        
        if (FlyBotHP <= 0)
        {
            DieExplosionAction(hit.collider.gameObject.transform.position);
            Destroy(hit.collider.gameObject);
            FlyBotHP = 20;
        }
    }
    void UnderattackExplosionAction(Vector3 position)
    {

        for (int i = 0; i < 15; i++)
        {
            Vector3 StartPos = position;
            GameObject ExplosObjects = Instantiate(ExplosionPreb, new Vector3(StartPos.x, StartPos.y, StartPos.z), Quaternion.identity);
            ExplosObjects.transform.Rotate(10, 10, 10);
            Rigidbody Exrigidby = ExplosObjects.GetComponent<Rigidbody>();
            // ·£´ýÇÑ ¹æÇâÀ¸·Î Æø¹ß
            Vector3 RandomDir = Random.insideUnitSphere * ParticleForce;
            RandomDir.x += 10.0f;
            Exrigidby.AddForce(RandomDir * 2, ForceMode.Impulse);
            // Lifetime ÃÊ µÚ¿¡ »ç¶óÁü
            Destroy(ExplosObjects, LifeTime);
        }

    }

    void DieExplosionAction(Vector3 position)
    {

        for (int i = 0; i < 50; i++)
        {
            Vector3 StartPos = position;
            GameObject ExplosObjects = Instantiate(longExplosionPreb, new Vector3(StartPos.x, StartPos.y, StartPos.z), Quaternion.identity);
            ExplosObjects.transform.Rotate(10, 10, 10);
            Rigidbody Exrigidby = ExplosObjects.GetComponent<Rigidbody>();
            // ·£´ýÇÑ ¹æÇâÀ¸·Î Æø¹ß
            Vector3 RandomDir = Random.insideUnitSphere * DieParticleForce;
    
            RandomDir.x-= 10.0f;
            Exrigidby.AddForce(RandomDir * 3, ForceMode.Impulse);
            // Lifetime ÃÊ µÚ¿¡ »ç¶óÁü
            Destroy(ExplosObjects, longLifeTime);
        }

    }
}
