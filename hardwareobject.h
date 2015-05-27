#ifndef HARDWAREOBJECT_H
#define HARDWAREOBJECT_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QApplication>
#include <QList>
#include <QPair>

#include "datastructs.h"
#include "communicationprotocol.h"
#include "scan.h"

/*!
 * \brief Abstract base class for all hardware connected to the instrument.
 *
 * This class establishes a common interface for all hardware.
 * Adding a new piece of hardware to the code involves creating a subclass of HardwareObject (or of one of the existing subclasses: see TcpInstrument, GpibInstrument, and Rs232Instrument)
 * Each hardware object has a name (d_prettyName and name()) that is used in the user interface, and a key (d_key and key()) that is used to refer to the object in the settings file.
 * Subclasses must assign strings to these variables in their constructors.
 * For communication with the UI, a logMessage() signal exists that can print a message in the log with an optional status code.
 * In general, this should only be used for reporting errors, but it is also useful in debugging.
 *
 * HardwareObjects are designed to be used in their own threads for the most part.
 * Because of this, the constructor should generally not be called with a parent, and any QObjects that will be created in the child class with this as the parent should NOT be initialized in the constructor.
 * Generally, a HardwareObject is created, signal-slot connections are made, and then the object is pushed into its own thread.
 * The thread's started() signal is connected to the initialize pure virtual slot, which must be implemented for a child class.
 * Any functions that need to be called from outside the class (e.g., from the HardwareManager during a scan) should be declared as slots, and QMetaObject::invokeMethod used to activate them (optionally with the Qt::BlockingQueuedConnection flag set if it returns a value or other actions need to wait until it is complete).
 * If for some reason this is not possible, then data members need to be protected by a QMutex to prevent concurrent access.
 * In the initialize function, any QObjects can be created and other settings made, because the object is already in its new thread.
 * The initialize function must call testConnection() before it returns.
 *
 * The testConnection() slot should attempt to establish communication with the physical device and perform some sort of test to make sure that the connection works and that the correct device is targeted.
 * Usually, this takes the form of an ID query of some type (e.g., *IDN?).
 * After determining if the connection is successful, the testConnection() function MUST emit the connectionResult() signal with the this pointer, a boolean indicating success, and an optional error message that can be displayed in the log along with the failure notification.
 * The testConnection() slot is also called from the communication dialog to re-test a connection that is lost.
 *
 * If at any point during operation, the program loses communication with the device, the hardwareFailure() signal should be emitted with the this pointer.
 * When emitted, any ongoing scan will be terminated and all controls will be frozen until communication is re-established by a successful call to the testConnection() function.
 * Some communication options (timeout, termination chartacters, etc) can be set, but it is up to the subclass to decide how or whether to make use of them.
 * They are provided here for convenience because TCP Instruments and RS232 instruments both use them.
 *
 * Finally, the sleep() function may be implemented in a subclass if anything needs to be done to put the device into an inactive state.
 * For instance, the FlowController turns off gas flows to conserve sample, and the PulseGenerator turns off all pulses.
 * If sleep() is implemented, it is recommneded to explicitly call HardwareObject::sleep(), as this will display a message in the log stating that the device is in fact asleep.
 */
class HardwareObject : public QObject
{
	Q_OBJECT
public:
    /*!
     * \brief Constructor. Does nothing.
     *
     * \param parent Pointer to parent QObject. Should be 0 if it will be in its own thread.
     */
    explicit HardwareObject(QObject *parent = 0);

    /*!
     * \brief Access function for pretty name.
     * \return Name for display on UI
     */
	QString name() { return d_prettyName; }

    /*!
     * \brief Access function for key.
     * \return Name for use in the settings file
     */
	QString key() { return d_key; }

    QString subKey() { return d_subKey; }

    bool isCritical() { return d_isCritical; }

    CommunicationProtocol::CommType type() { return p_comm->type(); }
	
signals:
    /*!
     * \brief Displays a message on the log.
     * \param QString The message to display
	* \param NicerOhms::MessageCode The status incidator (Normal, Warning, Error, Highlight)
     */
	void logMessage(const QString, const NicerOhms::LogMessageCode = NicerOhms::LogNormal);

    /*!
     * \brief Indicates whether a connection is successful
     * \param HardwareObject* This pointer
     * \param bool True if connection is successful
     * \param msg If unsuccessful, an optional message to display on the log tab along with the failure message.
     */
    void connected(bool success = true,QString msg = QString());

    /*!
     * \brief Signal emitted if communication to hardware is lost.
     * \param abort If an acquisition is underway, this will begin the abort routine if true
     */
    void hardwareFailure(bool abort = true);

    //data read at each scan point. to be plotted and logged to disk
    void pointDataRead(const QList<QPair<QString,QVariant>>);

    //data read at each scan point. Logged to disk, but not plotted
    void pointDataReadNoPlot(const QList<QPair<QString,QVariant>>);

	
public slots:
    /*!
     * \brief Attempt to communicate with hardware. Must emit connectionResult(). Pure virtual.
     * \return Whether attempt was successful
     */
	virtual bool testConnection() =0;

    /*!
     * \brief Do any needed initialization prior to connecting to hardware. Pure virtual
     *
     */
	virtual void initialize() =0;

    /*!
     * \brief Puts device into a standby mode. Default implementation puts a message in the log.
     * \param b If true, go into standby mode. Else, active mode.
     */
	virtual void sleep(bool b);

    //when this function is called, the hardware manager has already set the d_isActive variable
    //this function must call Scan::addNumDataPoints if it will be emitting any data (if !d_isActive, then this will be 0)
    //if some failure happens, call Scan::setHardwareFailure
    //return the scan object when done
    virtual Scan prepareForScan(Scan scan) =0;

    virtual void beginAcquisition() =0;
    virtual void endAcquisition() =0;
    virtual void readPointData() =0;
    void setActive(bool active) { d_isActive = active; }

protected:
    QString d_prettyName; /*!< Name to be displayed on UI */
    QString d_key; /*!< Name to be used in settings for abstract hardware*/
    QString d_subKey; /*< Name to be used in settings for real hardware*/

    CommunicationProtocol *p_comm;

    //if critical, scans cannot be run if device is disconnected
    //otherwise, scans can run, but some features may be disabled
    bool d_isCritical;

    //refers to whether a device is used during a scan
    bool d_isActive;


	
};

#endif // HARDWAREOBJECT_H
