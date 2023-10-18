using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FlyBot : MonoBehaviour
{


    private GameObject PlayerObject;
    public GameObject BulletObject;
    private float bulletSpeed = 500;
    private float BulletActiveTerm = 0.0f;
    //RayCast
    RaycastHit hit;
    // Start is called before the first frame update
    void Start()
    {
        PlayerObject = GameObject.Find("Ch44_nonPBR 1");
        
    }

    // Update is called once per frame
    void Update()
    {
       
        BulletActiveTerm += Time.deltaTime * 1.0f;
        transform.LookAt(PlayerObject.transform, Vector3.up);
        Vector3 playerPosition = PlayerObject.transform.position;

        if (Physics.Raycast(transform.position, transform.forward, out hit, 15.0f))
        {
            Vector3 moveDirection = playerPosition - transform.position;
            moveDirection.Normalize();

            transform.position += moveDirection * 1.5f * Time.deltaTime;
           transform.position = new Vector3(transform.position.x, 7.0f, transform.position.z);
            if (hit.collider.CompareTag("Player"))
            {
                BulletAction(transform.position, transform.forward);
            }

            //if (BulletActiveTerm > 2.0f)
            //{
            //    BulletAction(transform.position, transform.forward);
            //    BulletActiveTerm = 0.0f;
            //}
        }
    }

    void BulletAction(Vector3 spawnPosition, Vector3 direction)
    {

        GameObject bullet = Instantiate(BulletObject, new Vector3(spawnPosition.x, spawnPosition.y, spawnPosition.z), Quaternion.identity);
        bullet.transform.forward = direction;
        bullet.transform.Rotate(new Vector3(1, 0, 0), 90.0f);
        Rigidbody bulletRigidbody = bullet.GetComponent<Rigidbody>();
        bulletRigidbody.AddForce(direction * bulletSpeed, ForceMode.Force);
        Destroy(bullet, 2.0f);
    }
}
