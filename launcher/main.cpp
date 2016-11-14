#include <QApplication>
#include <QStyleFactory>
#include <QSharedMemory>
#include <QSystemSemaphore>

#include "launcherform.h"

void popup_existing_launcher()
{
    QSharedMemory popup("4668074d-98a4-4052-b8fb-0a3a7a84905c");
    if (popup.attach())
    {
        popup.lock();
        bool* is_needed = static_cast<bool*>(popup.data());
        *is_needed = true;
        popup.unlock();
        popup.detach();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Unique launcher
    QSystemSemaphore sema("5743c084-f257-472b-bd73-2837f34c0737", 1);
    bool isRunning;
    sema.acquire();

    {
        QSharedMemory shmem("50427dd1-0d94-42c8-a545-f1ddb0644c1c");
        shmem.attach();
    }

    QSharedMemory shmem("50427dd1-0d94-42c8-a545-f1ddb0644c1c");
    if (shmem.attach())
    {
        isRunning = true;
    }
    else
    {
        shmem.create(1);
        isRunning = false;
    }

    sema.release();
    // End unique laucnher

    if (isRunning)
    {
        popup_existing_launcher();
        return -1;
    }

    app.setStyle(QStyleFactory::create("fusion"));

    LauncherForm form;
    form.show();

    return app.exec();
}
