#include "../include/coroutine_utils.h"
#include "../include/eventloop.h"

QTNETWORKNG_NAMESPACE_BEGIN

void LambdaFunctor::operator ()()
{
    callback();
}


DeferCallThread::DeferCallThread(const std::function<void()> &func, YieldCurrentFunctor *yieldCoroutine, EventLoopCoroutine *eventloop)
:func(func), yieldCoroutine(yieldCoroutine), eventloop(eventloop)
{
}

void DeferCallThread::run()
{
    func();
    if(!eventloop.isNull()) {
        eventloop->callLaterThreadSafe(0, yieldCoroutine);
        eventloop->callLaterThreadSafe(100, new DeleteLaterFunctor<DeferCallThread>(this));
    } else {
        delete yieldCoroutine;
    }
}


CoroutineGroup::CoroutineGroup()
    :QObject()
{

}

CoroutineGroup::~CoroutineGroup()
{
    killall(true);
}

bool CoroutineGroup::add(QSharedPointer<Coroutine> coroutine, const QString &name)
{
    if(!name.isEmpty()) {
        if(!get(name).isNull()) {
            return false;
        }
        coroutine->setObjectName(name);
    }
    coroutine->finished.addCallback([this] (BaseCoroutine *coroutine) -> BaseCoroutine * {
        deleteCoroutine(coroutine);
        return coroutine;
    });
    coroutines.append(coroutine);
    return true;
}

QSharedPointer<Coroutine> CoroutineGroup::get(const QString &name)
{
    QListIterator<QSharedPointer<Coroutine>> itor(coroutines);
    while(itor.hasNext()) {
        QSharedPointer<Coroutine> coroutine = itor.next();
        if(coroutine->objectName() == name)
            return coroutine;
    }
    return QSharedPointer<Coroutine>();
}

bool CoroutineGroup::kill(const QString &name, bool join)
{
    QSharedPointer<Coroutine> found = get(name);
    if(!found.isNull()) {
        if(found.data() == Coroutine::current()) {
            qWarning("killing current coroutine?");
        } else {
            if(found->isRunning()) {
                found->kill();
            }
            if(join) {
                coroutines.removeOne(found);
                found->join();
            }
            return true;
        }
    }
    return false;
}

bool CoroutineGroup::killall(bool join)
{
    bool done = false;
    QList<QSharedPointer<Coroutine>> copy = coroutines;
    foreach(QSharedPointer<Coroutine> coroutine, copy) {
        if(coroutine.data() == Coroutine::current()) {
            qWarning() << "will not kill current coroutine while killall() is called:" << BaseCoroutine::current();
            continue;
        }
        if(coroutine->isRunning()) {
            coroutine->kill();
            done = true;
        }
    }

    if(join) {
        copy = coroutines;
        foreach(QSharedPointer<Coroutine> coroutine, copy) {
            if(coroutine.data() == Coroutine::current()) {
                qWarning("will not join current coroutine while killall() is called.");
                continue;
            }
            coroutines.removeOne(coroutine);
            coroutine->join();
        }
    }
    return done;
}

bool CoroutineGroup::joinall()
{
    bool hasCoroutines = !coroutines.isEmpty();
    QList<QSharedPointer<Coroutine>> copy = coroutines;
    foreach(QSharedPointer<Coroutine> coroutine, copy) {
        if(coroutine == Coroutine::current()) {
            qDebug("will not kill current coroutine while joinall() is called.");
            continue;
        }
        coroutines.removeOne(coroutine);
        coroutine->join();
    }
    return hasCoroutines;
}

struct DeleteCoroutineFunctor: public Functor
{
    virtual void operator()() {}
    QSharedPointer<BaseCoroutine> coroutine;
};

void CoroutineGroup::deleteCoroutine(BaseCoroutine *baseCoroutine)
{
    Coroutine *coroutine = dynamic_cast<Coroutine*>(baseCoroutine);
    Q_ASSERT(coroutine != 0);
    for(QList<QSharedPointer<Coroutine>>::iterator itor = coroutines.begin(); itor != coroutines.end(); ++itor) {
        if(itor->data() == coroutine) {
            DeleteCoroutineFunctor *callback = new DeleteCoroutineFunctor();
            callback->coroutine = *itor;
            EventLoopCoroutine::get()->callLater(0, callback);
            coroutines.erase(itor);
            break;
        }
    }
}

QTNETWORKNG_NAMESPACE_END
