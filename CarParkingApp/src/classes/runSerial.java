/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package classes;

import carparkingapp.settings;
import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

/**
 *
 * @author chamal
 */
public class runSerial {

    public static String sDataPattern = "", comPort;
    public static CommPortIdentifier portIdentifier;

    public void connectToSystem() throws Exception {

        if (settings.selectedStatus == 1) {
            
            comPort = settings.selectedComPort;

            try {
                portIdentifier = CommPortIdentifier.getPortIdentifier(comPort);
            } catch (Exception e) {
                JFrame frame = new JFrame("Warning");
                JOptionPane.showMessageDialog(frame,
                        "Wrong COM port selected. Select COM port from settings",
                        "Warning",
                        JOptionPane.WARNING_MESSAGE);
                frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
            }

            if (portIdentifier.isCurrentlyOwned()) {

                //JOptionPane interface
                JFrame frame = new JFrame("JOptionPane showMessageDialog example");
                JOptionPane.showMessageDialog(frame,
                        "Port is currently in use",
                        "Info",
                        JOptionPane.INFORMATION_MESSAGE);
                frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

            } else {
                CommPort commPort = portIdentifier.open(this.getClass().getName(), 2000);

                if (commPort instanceof SerialPort) {
                    SerialPort serialPort = (SerialPort) commPort;
                    serialPort.setSerialPortParams(9600, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

                    InputStream in = serialPort.getInputStream();
                    OutputStream out = serialPort.getOutputStream();

                    (new Thread(new runSerial.SerialReader(in))).start();
                    (new Thread(new runSerial.SerialWriter(out))).start();
                } else {

                    //JOptionPane interface
                    JFrame frame1 = new JFrame("JOptionPane showMessageDialog example");
                    JOptionPane.showMessageDialog(frame1,
                            "Only serial ports are handled.",
                            "Error",
                            JOptionPane.ERROR_MESSAGE);
                    frame1.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

                }
            }
        }else{
            
        }
    }

    /**
     *
     */
    public static class SerialReader implements Runnable {

        InputStream in;

        public SerialReader(InputStream in) {
            this.in = in;
        }

        public void run() {
            byte[] buffer = new byte[1024];
            int len = -1;

            try {
                while ((len = this.in.read(buffer)) > -1) {
                    //s = new String(buffer, 0, len);
                    sDataPattern = sDataPattern + new String(buffer, 0, len);
                    //System.out.print(s);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
    }

    /**
     *
     */
    public static class SerialWriter implements Runnable {

        OutputStream out;

        public SerialWriter(OutputStream out) {
            this.out = out;
        }

        public void run() {
            try {
                int c = 0;
                while ((c = System.in.read()) > -1) {
                    this.out.write(c);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static void mySystem() {
        try {
            runSerial balance = new runSerial();
            balance.connectToSystem();

        } catch (Exception e) {
            JFrame frame = new JFrame("Warning");
            JOptionPane.showMessageDialog(frame,
                    "Unidentified COM port selected, you'll be unable to get the weight reading",
                    "Warning",
                    JOptionPane.WARNING_MESSAGE);
            frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        }
    }

}
