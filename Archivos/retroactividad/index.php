<?php

include_once ("C:\AppServ/www/srh2/public_html/configuracion/config.php");    

include_once "config.php";

/**
 * Client Page indexObjeto
 * Client Page de inicio del paquete Objeto
 *
 * Fecha: 2003-01-13
 */

// Incluyendo la parte superior de la GUI
if ($HTTP_POST_VARS['formato']!="Plano") {
    include_once (PATH_GUI_REAL_TEMA."/gui_inicio.php");
    include_once (PATH_GUI_REAL_TEMA."/gui_arriba.php");
    include_once (PATH_GUI_REAL_TEMA."/gui_inicio_cuerpo.php");
}

$bd = new BD;
global $parametros;
$bd->conectar($parametros);

// Importando la clase del paquete
require_once "$objeto.inc";
require_once "$objeto_1.inc";
require_once "$objeto_2.inc";


$$objeto = new $objeto ($aplicacion,$modulo,$grupo,$usu_login,$bd);
$$objeto_1 = new $objeto_1 ($aplicacion,$modulo,$grupo,$usu_login,$bd);
$$objeto_2 = new $objeto_2 ($aplicacion,$modulo,$grupo,$usu_login,$bd);

switch ($_GET['accion']) {


}//END: switch ($_GET['accion'])


$$objeto->establecerOperacion($HTTP_POST_VARS['accion']);
$$objeto_1->establecerOperacion($HTTP_POST_VARS['accion']);
$$objeto_2->establecerOperacion($HTTP_POST_VARS['accion']);

$herr = new Herramienta;

if ($HTTP_POST_VARS['formato']!="Plano") {
  switch ($HTTP_POST_VARS['accion']) { // Aquí ya estoy seguro que la variable está en el '$HTTP_POST_VARS'

      default:
          print $herr->colocarTituloPaquete ($titulo="Manejo de la Nómina<br><br>");
      break;

  }
}

switch ($HTTP_POST_VARS['accion']) { // Aquí ya estoy seguro que la variable está en el '$HTTP_POST_VARS'

        case "Formulario Retroactividad":
                if(BLOQUEAR_LIQUIDACION){
                  echo "<script> alert('".BLOQUEAR_LIQUIDACION."');</script>";
                  exit;
                }
            $$objeto->mostrarFormulario($HTTP_POST_VARS);
        break;

        case "Liquidar Retroactividad":

			$persona_o_grupo = "";
			if($HTTP_POST_VARS['pel_grl_codigo'] == "")
				$persona_o_grupo = $HTTP_POST_VARS['grl_codigo'];
			else
				$persona_o_grupo = $HTTP_POST_VARS['pel_grl_codigo'];

			switch ($persona_o_grupo) {
				case 1:
					$$objeto_2->evaluarRetroactividadALiquidar($HTTP_POST_VARS);
				break;

				case 2:
					$$objeto_1->evaluarRetroactividadALiquidar($HTTP_POST_VARS);
				break;

				case 3:
					$$objeto->evaluarRetroactividadALiquidar($HTTP_POST_VARS);
				break;
			}

            //alvarela:2008-04-27
            //Para liquidar mes por mes, comentar la siguiene línea y
            //descomentar el include ("fm_SirhLiquidacionRetroactividadPeriodos.php");
            //del método evaluarRetroactividadALiquidar en retroactividad.inc
            if($HTTP_POST_VARS['tipo_aplicacion_retroactivo']=='GRUPOLIQUIDACION') {
				switch ($persona_o_grupo) {
					case 1:
						$$objeto_2->iniciarLiquidacionRetroactividad($HTTP_POST_VARS);
					break;

					case 2:
						$$objeto_1->iniciarLiquidacionRetroactividad($HTTP_POST_VARS);
					break;

					case 3:
						$$objeto->iniciarLiquidacionRetroactividad($HTTP_POST_VARS);
					break;
				}
			}

        break;

        case "Liquidar Retroactividad Mes":
            if ($HTTP_POST_VARS['radio'] != "")
            {
				switch ($HTTP_POST_VARS['pel_grl_codigo']) {
					case 1:
						$$objeto_2->iniciarLiquidacionRetroactividadD($HTTP_POST_VARS);
					break;
				
					case 2:
						$$objeto_1->iniciarLiquidacionRetroactividadD($HTTP_POST_VARS);
					break;
				
					case 3:
						$$objeto->iniciarLiquidacionRetroactividad($HTTP_POST_VARS);
					break;
				}
            }
            else
            {
                $error = new error("ERROR","ERROR896");
                $mesg = $error->retornarMensajeError();
                print "<center>$mesg</center><br><br>";
                switch ($HTTP_POST_VARS['pel_grl_codigo']) {
					case 1:
						$$objeto_2->evaluarRetroactividadALiquidarD($HTTP_POST_VARS);
					break;
					
					case 2:
						$$objeto_1->evaluarRetroactividadALiquidarD($HTTP_POST_VARS);
					break;
					
					case 3:
						$$objeto->evaluarRetroactividadALiquidar($HTTP_POST_VARS);
					break;
				}

            }
        break;

        case "ResumenRetro":
              $$objeto->mostrarFormularioResumenes($HTTP_POST_VARS);
        break;

        case "Generar Resumen":
              $$objeto->inicializarDatosResumen($HTTP_POST_VARS);
        break;
        
        case "MoverRetroactivo":
              $$objeto->mostrarFormulario_moverRetroactividad($HTTP_POST_VARS);
        break;
        
        case "Realizar Movimiento":
              $$objeto->moverRetroactividadANuevoPeriodo($HTTP_POST_VARS);     
              $$objeto->mostrarFormulario_moverRetroactividad($HTTP_POST_VARS);   
        break;
        
        


}

if ($HTTP_POST_VARS['formato']!="Plano") {
  //Incluyendo la parte Inferior de la GUI
  include_once (PATH_GUI_REAL_TEMA."/gui_final_cuerpo.php");
  include_once (PATH_GUI_REAL_TEMA."/gui_abajo.php");
  include_once (PATH_GUI_REAL_TEMA."/gui_terminar.php");
}


?>
