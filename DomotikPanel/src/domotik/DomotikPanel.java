package domotik;

import com.pi4j.io.serial.Baud;
import com.pi4j.io.serial.SerialDataEventListener;

import javax.swing.*;
import java.awt.*;
import java.io.IOException;

public class DomotikPanel {

    private JPanel panel;
    public JLabel timerLabel;
    public JLabel counterLabel;
    public JLabel comStatusLabel;

/*
    DomotikPanel(){
        initComponents();
    }


    private void initComponents() {

        panel = new JPanel();
        timerLabel = new JLabel();
        counterLabel = new JLabel();
        comStatusLabel = new JLabel();

        setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        setAlwaysOnTop(true);
        setBackground(new Color(255, 110, 140));
        setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
        setName("DomotikFrame"); // NOI18N
        setPreferredSize(Toolkit.getDefaultToolkit().getScreenSize());

        panel.setBackground(new Color(255, 110, 140));
        panel.setForeground(new Color(255, 110, 140));

        timerLabel.setFont(new Font("Tahoma", 1, 48)); // NOI18N
        timerLabel.setHorizontalAlignment(SwingConstants.CENTER);
        timerLabel.setText("100");
        timerLabel.setHorizontalTextPosition(SwingConstants.CENTER);
        timerLabel.setName("timerLabel"); // NOI18N

        counterLabel.setFont(new Font("Tahoma", 1, 76)); // NOI18N
        counterLabel.setHorizontalAlignment(SwingConstants.CENTER);
        counterLabel.setText("Counter");
        counterLabel.setToolTipText("");
        counterLabel.setHorizontalTextPosition(SwingConstants.CENTER);
        counterLabel.setName("counterLabel"); // NOI18N

        comStatusLabel.setText("status");
        comStatusLabel.setName("statusLabel"); // NOI18N


        GroupLayout jPanel1Layout = new GroupLayout(panel);
        panel.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
                jPanel1Layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                        .addComponent(timerLabel, GroupLayout.Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(comStatusLabel, GroupLayout.Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(counterLabel, GroupLayout.Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(jPanel1Layout.createSequentialGroup()
                                .addGap(269, 269, 269)
                                .addContainerGap(298, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
                jPanel1Layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                        .addGroup(GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                                .addContainerGap()
                                .addComponent(timerLabel, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE)
                                .addGap(18, 18, 18)
                                .addComponent(counterLabel, GroupLayout.PREFERRED_SIZE, 192, GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(LayoutStyle.ComponentPlacement.RELATED, 11, Short.MAX_VALUE)
                                .addGap(18, 18, 18)
                                .addComponent(comStatusLabel))
        );

        GroupLayout layout = new GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
                layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                        .addComponent(panel, GroupLayout.Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
                layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                        .addComponent(panel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        getAccessibleContext().setAccessibleName("DomotikPanel");
        getAccessibleContext().setAccessibleDescription("");

        pack();
    }
*/
/*
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
    //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
    /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
     * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html
     */
        /*try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(DomotikPanel.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(DomotikPanel.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(DomotikPanel.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(DomotikPanel.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        DomotikPanel frameDomotik = new DomotikPanel();

        int w = Toolkit.getDefaultToolkit().getScreenSize().width;
        int h = Toolkit.getDefaultToolkit().getScreenSize().height;
        Point p = frameDomotik.panel.getLocation();
        frameDomotik.timerLabel.setLocation(w/3, h/2);
        frameDomotik.counterLabel.setLocation(p.x + w/2, p.y + h/2);
        frameDomotik.comStatusLabel.setLocation(w/3*2, h/2);
        frameDomotik.setPreferredSize(Toolkit.getDefaultToolkit().getScreenSize());
        frameDomotik.setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        frameDomotik.pack();
        */

/*        Serial serial = new Serial("/dev/rfcomm0", Baud._9600);

        serial.addReaderListeners((SerialDataEventListener) serialDataEvent -> {
            try {

                String serialBuffer = serialDataEvent.getAsciiString();

                //Texte attendu: "chrono;score"
                String[] myArgs = serialBuffer.split(";");
                frameDomotik.pointCounterLabel.setText(myArgs[1]);
                frameDomotik.timerLabel.setText(myArgs[0]);

                System.out.println("Position : " + serialBuffer);
            } catch (IOException e) {
                System.out.println("Echec du parsing de la position : " + e.getMessage());
            }
        });
*/


    /* Create and display the form */
        /*java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                frameDomotik.setVisible(true);

            }
        });
    }
    */

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Too few argumment CONNARD !!! (comPort baudRate)");
            return;
        }

        //init Serial
        String comPort = args[0];
        String baud = args[1];
        Baud baudRate = Baud.getInstance(Integer.valueOf(baud));
        JFrame frameDomotik = new JFrame("DomotikFrame");
        //Serial serial = new Serial("/dev/rfcomm0", Baud._9600);
        Serial serial = new Serial(comPort, baudRate);

        //Init graphical part
        DomotikPanel myPanel = new DomotikPanel();
        frameDomotik.setContentPane(myPanel.panel);
        frameDomotik.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frameDomotik.setPreferredSize(Toolkit.getDefaultToolkit().getScreenSize());

        frameDomotik.pack();
        frameDomotik.setVisible(true);


        serial.addReaderListeners((SerialDataEventListener) serialDataEvent -> {
            try {

                String serialBuffer = serialDataEvent.getAsciiString();

                //Texte attendu: "chrono;score"
                String[] myArgs = serialBuffer.split(";");
                myPanel.counterLabel.setText("Score:" + myArgs[1]);
                myPanel.timerLabel.setText(myArgs[0]);


                System.out.println("Received : " + serialBuffer);
            } catch (IOException e) {
                System.out.println("Echec du parsing de la position : " + e.getMessage());
            }
        });


    }


    {
// GUI initializer generated by IntelliJ IDEA GUI Designer
// >>> IMPORTANT!! <<<
// DO NOT EDIT OR ADD ANY CODE HERE!
        $$$setupUI$$$();
    }

    /**
     * Method generated by IntelliJ IDEA GUI Designer
     * >>> IMPORTANT!! <<<
     * DO NOT edit this method OR call it in your code!
     *
     * @noinspection ALL
     */
    private void $$$setupUI$$$() {
        panel = new JPanel();
        panel.setLayout(new com.intellij.uiDesigner.core.GridLayoutManager(3, 2, new Insets(0, 0, 0, 0), -1, -1));
        panel.setBackground(new Color(-754475));
        panel.setForeground(new Color(-754475));
        timerLabel = new JLabel();
        Font timerLabelFont = this.$$$getFont$$$(null, -1, 72, timerLabel.getFont());
        if (timerLabelFont != null) timerLabel.setFont(timerLabelFont);
        timerLabel.setForeground(new Color(-1));
        timerLabel.setText("Label");
        panel.add(timerLabel, new com.intellij.uiDesigner.core.GridConstraints(0, 0, 1, 2, com.intellij.uiDesigner.core.GridConstraints.ANCHOR_CENTER, com.intellij.uiDesigner.core.GridConstraints.FILL_NONE, com.intellij.uiDesigner.core.GridConstraints.SIZEPOLICY_FIXED, com.intellij.uiDesigner.core.GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        counterLabel = new JLabel();
        Font counterLabelFont = this.$$$getFont$$$(null, Font.BOLD, 72, counterLabel.getFont());
        if (counterLabelFont != null) counterLabel.setFont(counterLabelFont);
        counterLabel.setForeground(new Color(-1));
        counterLabel.setText("Counter");
        panel.add(counterLabel, new com.intellij.uiDesigner.core.GridConstraints(1, 0, 1, 2, com.intellij.uiDesigner.core.GridConstraints.ANCHOR_CENTER, com.intellij.uiDesigner.core.GridConstraints.FILL_NONE, com.intellij.uiDesigner.core.GridConstraints.SIZEPOLICY_FIXED, com.intellij.uiDesigner.core.GridConstraints.SIZEPOLICY_WANT_GROW, null, new Dimension(37, 242), null, 0, false));
        comStatusLabel = new JLabel();
        comStatusLabel.setText("Label");
        panel.add(comStatusLabel, new com.intellij.uiDesigner.core.GridConstraints(2, 0, 1, 1, com.intellij.uiDesigner.core.GridConstraints.ANCHOR_CENTER, com.intellij.uiDesigner.core.GridConstraints.FILL_NONE, com.intellij.uiDesigner.core.GridConstraints.SIZEPOLICY_FIXED, com.intellij.uiDesigner.core.GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
    }

    /**
     * @noinspection ALL
     */
    private Font $$$getFont$$$(String fontName, int style, int size, Font currentFont) {
        if (currentFont == null) return null;
        String resultName;
        if (fontName == null) {
            resultName = currentFont.getName();
        } else {
            Font testFont = new Font(fontName, Font.PLAIN, 10);
            if (testFont.canDisplay('a') && testFont.canDisplay('1')) {
                resultName = fontName;
            } else {
                resultName = currentFont.getName();
            }
        }
        return new Font(resultName, style >= 0 ? style : currentFont.getStyle(), size >= 0 ? size : currentFont.getSize());
    }

    /**
     * @noinspection ALL
     */
    public JComponent $$$getRootComponent$$$() {
        return panel;
    }
}
