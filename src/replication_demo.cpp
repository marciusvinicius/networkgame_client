/*
 * AOI + Dirty Flag Replication Example in C
 * Simulates server-side update + selective replication per client
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define MAX_ENTITIES 10
#define MAX_CLIENTS 4
#define AOI_RADIUS 25.0f

#define DIRTY_POS (1 << 0)
#define DIRTY_HEALTH (1 << 1)

typedef struct
{
    float x, y;
} Vec2;

typedef struct
{
    int id;
    Vec2 position;
    float health;
    uint8_t dirty_flags;
} Entity;

typedef struct
{
    int id;
    Vec2 position;
    int observed_entities[MAX_ENTITIES];
    int num_observed;
} Client;

void set_position(Entity *e, float x, float y)
{
    if (e->position.x != x || e->position.y != y)
    {
        e->position.x = x;
        e->position.y = y;
        e->dirty_flags |= DIRTY_POS;
    }
}

void set_health(Entity *e, float hp)
{
    if (e->health != hp)
    {
        e->health = hp;
        e->dirty_flags |= DIRTY_HEALTH;
    }
}

void update_aoi(Client *c, Entity *entities, int num_entities)
{
    c->num_observed = 0;
    for (int i = 0; i < num_entities; i++)
    {
        float dx = c->position.x - entities[i].position.x;
        float dy = c->position.y - entities[i].position.y;
        if ((dx * dx + dy * dy) <= AOI_RADIUS * AOI_RADIUS)
        {
            c->observed_entities[c->num_observed++] = i;
        }
    }
}

void serialize_entity(const Entity *e, uint8_t *buffer, int *offset)
{
    buffer[(*offset)++] = (uint8_t)e->id;
    buffer[(*offset)++] = e->dirty_flags;

    if (e->dirty_flags & DIRTY_POS)
    {
        memcpy(&buffer[*offset], &e->position, sizeof(Vec2));
        *offset += sizeof(Vec2);
    }
    if (e->dirty_flags & DIRTY_HEALTH)
    {
        memcpy(&buffer[*offset], &e->health, sizeof(float));
        *offset += sizeof(float);
    }
}

void replicate(Client *c, Entity *entities)
{
    uint8_t buffer[256];
    int offset = 0;

    for (int i = 0; i < c->num_observed; i++)
    {
        int ent_id = c->observed_entities[i];
        const Entity *e = &entities[ent_id];
        if (e->dirty_flags)
        {
            serialize_entity(e, buffer, &offset);
        }
    }

    printf("Client %d replication packet (%d bytes):\n", c->id, offset);
    for (int i = 0; i < offset; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");
}

int main()
{
    Entity entities[MAX_ENTITIES];
    Client clients[MAX_CLIENTS];

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i].id = i;
        entities[i].position = (Vec2){i * 10.0f, 0};
        entities[i].health = 100.0f;
        entities[i].dirty_flags = 0;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].id = i;
        clients[i].position = (Vec2){i * 20.0f, 0};
    }

    // Simulate a change
    set_position(&entities[2], 22.0f, 0);
    set_health(&entities[4], 80.0f);

    // Update AOI + Replicate
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        update_aoi(&clients[i], entities, MAX_ENTITIES);
        replicate(&clients[i], entities);
    }

    // Clear dirty flags after replication
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i].dirty_flags = 0;
    }

    return 0;
}
