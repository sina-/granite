#include <oni-core/network/oni-network-server.h>

#include <enet/enet.h>

#include <oni-core/entities/oni-entities-manager.h>
#include <oni-core/entities/oni-entities-factory.h>
#include <oni-core/entities/oni-entities-serialization.h>
#include <oni-core/component/oni-component-audio.h>


namespace oni {
    namespace network {
        Server::Server(const Address *address,
                       common::u8 numClients,
                       common::u8 numChannels) :
                Peer::Peer(address, numClients, numChannels, 0, 0) {
        }

        Server::Server() = default;

        Server::~Server() = default;

        void
        Server::postConnectHook(const ENetEvent *event) {
        }

        void
        Server::postDisconnectHook(const ENetEvent *event) {
            auto clientID = getPeerID(*event->peer);

            mPostDisconnectHook(clientID);
        }

        void
        Server::handle(ENetPeer *peer,
                       enet_uint8 *data,
                       size_t size,
                       PacketType header) {
            auto peerID = getPeerID(*peer);
            assert(mPacketHandlers.find(header) != mPacketHandlers.end() || header == PacketType::PING ||
                   header == PacketType::MESSAGE);
            switch (header) {
                case (PacketType::PING): {
                    auto type = PacketType::PING;
                    auto pingData = std::string{};

                    send(type, pingData, peer);

                    break;
                }
                case (PacketType::MESSAGE): {
                    auto packet = entities::deserialize<Packet_Data>(data, size);
                    break;
                }
                case (PacketType::SETUP_SESSION): {
                    mPacketHandlers[PacketType::SETUP_SESSION](peerID, "");
                    break;
                }
                case (PacketType::CLIENT_INPUT): {
                    auto dataString = std::string(reinterpret_cast<char *>(data), size);
                    mPacketHandlers[PacketType::CLIENT_INPUT](peerID, dataString);
                    break;
                }
                default: {
                    // TODO: Need to keep stats on clients with bad packets and block them when threshold reaches.
                    assert(false);
                    break;
                }
            }
        }

        void
        Server::sendEntitiesAll(entities::EntityManager &manager) {
            std::string data = entities::serialize(manager, entities::SnapshotType::ENTIRE_REGISTRY);
            auto type = PacketType::REGISTRY_REPLACE_ALL_ENTITIES;

            if (data.size() > 1) {
                broadcast(type, data);
            }
        }

        void
        Server::sendComponentsUpdate(entities::EntityManager &manager) {
            // TODO: What happens if broadcast fails for some clients? Would they miss these entities forever?
            std::string data = entities::serialize(manager, entities::SnapshotType::ONLY_COMPONENTS);
            auto type = PacketType::REGISTRY_ONLY_COMPONENT_UPDATE;

            if (data.size() > 1) {
                broadcast(type, data);
            }
        }

        void
        Server::sendNewEntities(entities::EntityManager &manager) {
            // TODO: What happens if broadcast fails for some clients? Would they miss these entities forever?
            std::string data = entities::serialize(manager, entities::SnapshotType::ONLY_NEW_ENTITIES);
            auto type = PacketType::REGISTRY_ADD_NEW_ENTITIES;

            if (data.size() > 1) {
                broadcast(type, data);
            }
        }

        void
        Server::broadcastDeletedEntities(entities::EntityManager &manager) {
            auto type = PacketType::REGISTRY_DESTROYED_ENTITIES;
            auto data = entities::serialize(manager.getDeletedEntities());
            manager.clearDeletedEntitiesList();

            if (data.size() > 1) {
                broadcast(type, data);
            }
        }

        void
        Server::sendCarEntityID(common::EntityID entityID,
                                const common::PeerID &peerID) {
            auto packet = Packet_EntityID{entityID};
            auto data = entities::serialize(packet);
            auto type = PacketType::CAR_ENTITY_ID;

            send(type, data, mPeers[peerID]);
        }

        void
        Server::handleEvent_Collision(const oni::game::Event_Collision &event) {
            auto data = oni::entities::serialize(event);
            auto packetType = oni::network::PacketType::EVENT_COLLISION;

            queueForBroadcast(packetType, data);
        }

        void
        Server::handleEvent_SoundPlay(const oni::game::Event_SoundPlay &event) {
            auto data = oni::entities::serialize(event);
            auto packetType = oni::network::PacketType::EVENT_SOUND_PLAY;

            queueForBroadcast(packetType, data);
        }

        void
        Server::handleEvent_RocketLaunch(const oni::game::Event_RocketLaunch &event) {
            auto data = oni::entities::serialize(event);
            auto packetType = oni::network::PacketType::EVENT_ROCKET_LAUNCH;

            queueForBroadcast(packetType, data);
        }
    }
}