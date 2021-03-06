#ifndef DATABASEBASE_H
#define DATABASEBASE_H

typedef void (*CallBackDatabase)(void *object);

namespace CatchChallenger {
class DatabaseBase
{
    public:
        struct CallBack
        {
            void *object;
            CallBackDatabase method;
        };
        virtual bool syncConnect(const char * host, const char * dbname, const char * user, const char * password) = 0;
        virtual void syncDisconnect() = 0;
        virtual CallBack * asyncRead(const char *query,void * returnObject,CallBackDatabase method) = 0;
        virtual bool asyncWrite(const char *query) = 0;
        virtual const char * errorMessage() const = 0;
        virtual bool next() = 0;
        virtual const char * value(const int &value) const = 0;
        virtual bool isConnected() const = 0;
};
}

#endif // DATABASEBASE_H
